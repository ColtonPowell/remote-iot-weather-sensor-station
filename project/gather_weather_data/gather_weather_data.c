// IoT Real-Time Weather Station
// Colton Powell, Yezhou Zhao, Matthew Salmanpour, Justin Visas
//------------------------------------------------------------
#include "wiced.h"
#include "mqtt_api.h"
#include "resources.h"
#include "wiced_defaults.h"
#include "wwd_debug.h"
#include "crypto_api.h"

/******************************************************
 *                      Macros
 ******************************************************/
/* CHANGE MQTT_BROKER_ADDRESS to the IP address of your MQTT broker */
#define MQTT_BROKER_ADDRESS                 "34.217.176.36"
#define WICED_TOPIC                         "Stats"
/* CHANGE CLIENT_ID to a unique id for your weather station 
 * The message sent via MQTT is limited in size so don't make 
 * it too long or else you may lose/corrupt your data sent */
#define CLIENT_ID                           "IoT_Device_Pat"
#define MQTT_REQUEST_TIMEOUT                (5000)
#define MQTT_DELAY_IN_MILLISECONDS          (1000)
#define MQTT_PUBLISH_RETRY_COUNT            (20)
#define MQTT_SUBSCRIBE_RETRY_COUNT          (3)
#define AES_KEY_LENGTH                      128

/* Shield to board stuff */
#define I2C_ADDRESS         (0x42)
#define RETRIES             (1)
#define DISABLE_DMA         (WICED_TRUE)
#define NUM_MESSAGES        (1)
#define TEMPERATURE_REG     0x07

/******************************************************
 *               Variable Definitions
 ******************************************************/
static wiced_ip_address_t                   broker_address;
static wiced_mqtt_event_type_t              received_event;
static wiced_semaphore_t                    event_semaphore;
static wiced_semaphore_t                    wake_semaphore;
static uint8_t                              pub_in_progress = 0;
static uint8_t                              aes_key[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

/******************************************************
 *               Static Function Definitions
 ******************************************************/

/*
 * A blocking call to an expected event.
 */
static wiced_result_t wait_for_response( wiced_mqtt_event_type_t excpected_event, uint32_t timeout )
{
    if ( wiced_rtos_get_semaphore( &event_semaphore, timeout ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    else
    {
        if ( excpected_event != received_event )
        {
            return WICED_ERROR;
        }
    }
    return WICED_SUCCESS;
}

/*
 * Call back function to handle connection events.
 */
static wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event )
{
    static char data[ 30 ];

    switch ( event->type )
    {
    case WICED_MQTT_EVENT_TYPE_SUBCRIBED:
    {
        WPRINT_APP_INFO(( "\nSubscription acknowledged!\n" ));
        received_event = event->type;
        wiced_rtos_set_semaphore( &event_semaphore );
    }
        break;
    case WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS:
    case WICED_MQTT_EVENT_TYPE_DISCONNECTED:
    case WICED_MQTT_EVENT_TYPE_PUBLISHED:
    case WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED:
    {
        received_event = event->type;
        wiced_rtos_set_semaphore( &event_semaphore );
    }
        break;
    case WICED_MQTT_EVENT_TYPE_PUBLISH_MSG_RECEIVED:
    {
        WPRINT_APP_INFO(( "\nReceived message from broker!\n" ));
        wiced_mqtt_topic_msg_t msg = event->data.pub_recvd;
        memcpy( data, msg.data, msg.data_len );
        data[ msg.data_len + 1 ] = '\0';
        if ( !strncmp( data, "LIGHT ON", 8 ) )
        {
            wiced_gpio_output_high( WICED_SH_LED1 );
            WPRINT_APP_INFO(( "LIGHT ON\n" ));
        }
        else
        {
            wiced_gpio_output_low( WICED_SH_LED1 );
            WPRINT_APP_INFO(( "LIGHT OFF\n" ));
        }
    }
        break;
    default:
        break;
    }
    return WICED_SUCCESS;
}

/*
 * Open a connection and wait for MQTT_REQUEST_TIMEOUT period to receive a connection open OK event
 */
static wiced_result_t mqtt_conn_open( wiced_mqtt_object_t mqtt_obj, wiced_ip_address_t *address, wiced_interface_t interface, wiced_mqtt_callback_t callback, wiced_mqtt_security_t *security )
{
    wiced_mqtt_pkt_connect_t conninfo;
    wiced_result_t ret = WICED_SUCCESS;

    memset( &conninfo, 0, sizeof( conninfo ) );
    conninfo.port_number = 1883;
    conninfo.mqtt_version = WICED_MQTT_PROTOCOL_VER4;
    conninfo.clean_session = 1;
    conninfo.client_id = (uint8_t*) CLIENT_ID;
    conninfo.keep_alive = 5;
    conninfo.username = (uint8_t*)"coltons_board";
    conninfo.password = (uint8_t*)"coen";
    ret = wiced_mqtt_connect( mqtt_obj, address, interface, callback, security, &conninfo );
    if ( ret != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    if ( wait_for_response( WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS, MQTT_REQUEST_TIMEOUT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/*
 * Subscribe to WICED_TOPIC and wait for 5 seconds to receive an ACK.
 */
static wiced_result_t mqtt_app_subscribe( wiced_mqtt_object_t mqtt_obj, char *topic, uint8_t qos )
{
    wiced_mqtt_msgid_t pktid;
    pktid = wiced_mqtt_subscribe( mqtt_obj, topic, qos );
    if ( pktid == 0 )
    {
        return WICED_ERROR;
    }
    if ( wait_for_response( WICED_MQTT_EVENT_TYPE_SUBCRIBED, MQTT_REQUEST_TIMEOUT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/*
 * Publish (send) message to WICED_TOPIC and wait for 5 seconds to receive a PUBCOMP (as it is QoS=2).
 */
static wiced_result_t mqtt_app_publish( wiced_mqtt_object_t mqtt_obj, uint8_t qos, uint8_t *topic, uint8_t *data, uint32_t data_len )
{
    wiced_mqtt_msgid_t pktid;

    pktid = wiced_mqtt_publish( mqtt_obj, topic, data, data_len, qos );

    if ( pktid == 0 )
    {
        return WICED_ERROR;
    }

    if ( wait_for_response( WICED_MQTT_EVENT_TYPE_PUBLISHED, MQTT_REQUEST_TIMEOUT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/*
 * Close a connection and wait for 5 seconds to receive a connection close OK event
 */
static wiced_result_t mqtt_conn_close( wiced_mqtt_object_t mqtt_obj )
{
    if ( wiced_mqtt_disconnect( mqtt_obj ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    if ( wait_for_response( WICED_MQTT_EVENT_TYPE_DISCONNECTED, MQTT_REQUEST_TIMEOUT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/******************************************************
 *               Function Definitions
 ******************************************************/
void application_start( void )
{
    wiced_mqtt_object_t   mqtt_object;
    wiced_result_t        ret = WICED_SUCCESS;
    int                   connection_retries = 0;
    int                   pub_sub_retries = 0;
    char                 mqtt_msg[128];
    char*                clr_txt;
    uint8_t*             cphr_txt;
    uint8_t              iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t             iv_offset;
    hw_aes_context_t ctx;

    wiced_init( );

    /* Encryption */
    platform_hwcrypto_init( );
    hw_aes_setkey_enc(&ctx, (unsigned char*)aes_key, AES_KEY_LENGTH);

    /* Disable roaming to other access points */
    wiced_wifi_set_roam_trigger( -99 ); /* -99dBm ie. extremely low signal level */

    /* Bring up the network interface */
    ret = wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
    if ( ret != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "\nNot able to join the requested AP\n\n" ) );
        return;
    }

    /* Allocate memory for MQTT object*/
    mqtt_object = (wiced_mqtt_object_t) malloc( WICED_MQTT_OBJECT_MEMORY_SIZE_REQUIREMENT );
    if ( mqtt_object == NULL )
    {
        WPRINT_APP_ERROR("Don't have memory to allocate for MQTT object...\n");
        return;
    }

    /* Resolve and print IP address of the specified MQTT broker*/
    WPRINT_APP_INFO( ( "Resolving IP address of MQTT broker...\n" ) );
    ret = wiced_hostname_lookup( MQTT_BROKER_ADDRESS, &broker_address, 10000 , WICED_STA_INTERFACE);
    WPRINT_APP_INFO(("Resolved Broker IP: %u.%u.%u.%u\n\n", (uint8_t)(GET_IPV4_ADDRESS(broker_address) >> 24),
            (uint8_t)(GET_IPV4_ADDRESS(broker_address) >> 16),
            (uint8_t)(GET_IPV4_ADDRESS(broker_address) >> 8),
            (uint8_t)(GET_IPV4_ADDRESS(broker_address) >> 0)));
    if ( ret == WICED_ERROR || broker_address.ip.v4 == 0 )
    {
        WPRINT_APP_INFO(("Error in resolving DNS\n"));
        return;
    }

    /* SETUP I2C */
    const wiced_i2c_device_t i2cDevice = {
            .port = WICED_I2C_2,
            .address = I2C_ADDRESS,
            .address_width = I2C_ADDRESS_WIDTH_7BIT,
            .speed_mode = I2C_STANDARD_SPEED_MODE
    };
    wiced_i2c_init(&i2cDevice);

    /* Tx buffer is used to set the offset */
    uint8_t tx_buffer[] = {TEMPERATURE_REG};
    wiced_i2c_message_t setOffset;
    wiced_i2c_init_tx_message(&setOffset, tx_buffer,
    sizeof(tx_buffer), RETRIES, DISABLE_DMA);

    /* Initialize offset */
    wiced_i2c_transfer(&i2cDevice, &setOffset, NUM_MESSAGES);

    /* Rx buffer is used to get temperature, humidity, light, and POT data */
    struct {
        float temp;
        float humidity;
        float light;
        float pot;
    } rx_buffer;

    /* The message to be sent from the shield to the board via i2c */
    wiced_i2c_message_t i2c_msg;
    wiced_i2c_init_rx_message(&i2c_msg, &rx_buffer, sizeof(rx_buffer), RETRIES, DISABLE_DMA);

    wiced_rtos_init_semaphore( &wake_semaphore );
    wiced_mqtt_init( mqtt_object );
    wiced_rtos_init_semaphore( &event_semaphore );

    do //note: reconnection is performed whenever the initial loop fails to run
    {
        // Attempt to open a connection for WICED_MQTT_CONNECTION_NUMBER_OF_RETRIES times
        WPRINT_APP_INFO(("[MQTT] Opening connection..."));
        do
        {
            WPRINT_APP_INFO(("[MQTT] Attempt %d to connect ...", connection_retries));
            ret = mqtt_conn_open( mqtt_object, &broker_address, WICED_STA_INTERFACE, mqtt_connection_event_cb, NULL );
            connection_retries++ ;
            wiced_rtos_delay_milliseconds( 1000 );
        } while ( ( ret != WICED_SUCCESS ) && ( connection_retries < 20) );

        if ( ret != WICED_SUCCESS )
        {
            WPRINT_APP_INFO(("Failed connection!\n"));
            break;
        }

        /* Subscription has been successfully established by now */
        while ( 1 )
        {
            wiced_rtos_set_semaphore( &wake_semaphore );
            wiced_rtos_get_semaphore( &wake_semaphore, WICED_NEVER_TIMEOUT );

            /* Get data from I2C */
            wiced_i2c_transfer(&i2cDevice, &i2c_msg, NUM_MESSAGES);
            /* And store it in mqtt_msg */
            sprintf(mqtt_msg, "                    \nID:%s\nTemperature:%.2f\nHumidity:%.2f\nLight:%.2f\nPOT:%.2f",
                    CLIENT_ID, rx_buffer.temp, rx_buffer.humidity,
                    rx_buffer.light, rx_buffer.pot);

            clr_txt = (char *)calloc(sizeof(uint8_t), 128);
            cphr_txt = (uint8_t *)calloc(sizeof(uint8_t),128); 
            sprintf(clr_txt, "%s", mqtt_msg);

            hw_aes_crypt_cfb(&ctx, HW_AES_ENCRYPT, 128, &iv_offset, (unsigned char*)iv, (unsigned char*)clr_txt, cphr_txt);

            WPRINT_APP_INFO((mqtt_msg));
            //WPRINT_APP_INFO((cphr_txt));

            WPRINT_APP_INFO(("[MQTT] Publishing..."));

            /* reset pub_sub_retries to 0 before going into the loop so 
	       that the next publish after a failure will still work */
            pub_sub_retries = 0;

            /* Now attempt to publish the message for MQTT_PUBLISH_RETRY_COUNT times */
            do
            {
	      // ret = mqtt_app_publish( mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) WICED_TOPIC, (uint8_t*) mqtt_msg, 128 );
                ret = mqtt_app_publish( mqtt_object, WICED_MQTT_QOS_DELIVER_AT_LEAST_ONCE, (uint8_t*) WICED_TOPIC, (uint8_t*) cphr_txt, 128 );

                pub_sub_retries++ ;
            } while ( ( ret != WICED_SUCCESS ) && ( pub_sub_retries < MQTT_PUBLISH_RETRY_COUNT ) );
            if ( ret != WICED_SUCCESS )
            {
                WPRINT_APP_INFO((" Failed publishing!\n"));
                break; // break the loop and reconnect
            }
            else
            {
                WPRINT_APP_INFO((" Successful publishing!\n"));
            }

            /* Publish successful, wait 1 second until next transmission */
	    // free(clr_txt);
	    // free(cphr_txt);
            wiced_rtos_delay_milliseconds( 1000 );
        }

        WPRINT_APP_INFO(("[MQTT] Closing connection..."));
        mqtt_conn_close( mqtt_object );

        wiced_rtos_delay_milliseconds( MQTT_DELAY_IN_MILLISECONDS * 2 );
    } while ( 1 );

    wiced_rtos_deinit_semaphore( &event_semaphore );
    WPRINT_APP_INFO(("[MQTT] Deinit connection...\n"));
    ret = wiced_mqtt_deinit( mqtt_object );
    wiced_rtos_deinit_semaphore( &wake_semaphore );
    free( mqtt_object );
    mqtt_object = NULL;

    return;
}
