#include "BLE_Driver.h"
#include <string.h>
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "nvs_flash.h"

#define BLE_APP_ID       0x40
#define BLE_SERVICE_UUID 0x00FF
#define BLE_RX_UUID      0xFF01
#define BLE_TX_UUID      0xFF02

static const char *s_name = "ESP32-S3-DevKitC-1";
static uint16_t s_service_handle = 0;
static uint16_t s_rx_handle = 0;
static uint16_t s_tx_handle = 0;

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t char_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint8_t char_prop_rw = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static uint8_t rx_value[20] = {0};
static uint8_t tx_value[20] = "ESP32-S3 BLE";

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    (void)param;
    if (event == ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT) {
        esp_ble_adv_params_t adv = {
            .adv_int_min = 0x20,
            .adv_int_max = 0x40,
            .adv_type = ADV_TYPE_IND,
            .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
            .channel_map = ADV_CHNL_ALL,
            .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
        };
        esp_ble_gap_start_advertising(&adv);
    }
}

static void gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                     esp_ble_gatts_cb_param_t *p)
{
    switch (event) {
    case ESP_GATTS_REG_EVT: {
        esp_ble_gap_set_device_name(s_name);
        esp_ble_gap_config_adv_data(&(esp_ble_adv_data_t){
            .set_scan_rsp = false,
            .include_name = true,
            .include_txpower = true,
            .flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT,
        });

        esp_gatt_srvc_id_t service_id = {0};
        service_id.is_primary = true;
        service_id.id.inst_id = 0;
        service_id.id.uuid.len = ESP_UUID_LEN_16;
        service_id.id.uuid.uuid.uuid16 = BLE_SERVICE_UUID;
        esp_ble_gatts_create_service(gatts_if, &service_id, 8);
        break;
    }
    case ESP_GATTS_CREATE_EVT:
        s_service_handle = p->create.service_handle;
        esp_ble_gatts_start_service(s_service_handle);
        esp_bt_uuid_t rx_uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid.uuid16 = BLE_RX_UUID,
        };
        esp_ble_gatts_add_char(s_service_handle, &rx_uuid,
                               ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                               char_prop_rw, rx_value, NULL);
        break;
    case ESP_GATTS_ADD_CHAR_EVT: {
        uint16_t uuid = p->add_char.char_uuid.uuid.uuid16;
        if (uuid == BLE_RX_UUID) {
            s_rx_handle = p->add_char.attr_handle;
            esp_bt_uuid_t tx_uuid = {
                .len = ESP_UUID_LEN_16,
                .uuid.uuid16 = BLE_TX_UUID,
            };
            esp_ble_gatts_add_char(s_service_handle, &tx_uuid,
                                   ESP_GATT_PERM_READ,
                                   char_prop_notify, tx_value, NULL);
        } else if (uuid == BLE_TX_UUID) {
            s_tx_handle = p->add_char.attr_handle;
        }
        break;
    }
    case ESP_GATTS_CONNECT_EVT:
    case ESP_GATTS_DISCONNECT_EVT:
    case ESP_GATTS_WRITE_EVT:
    case ESP_GATTS_READ_EVT:
    default:
        break;
    }
}

esp_err_t BLE_Init(const BLE_InitTypeDef *c)
{
    if (!c || !c->DeviceName) return ESP_ERR_INVALID_ARG;
    s_name = c->DeviceName;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) return ret;

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) return ret;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
    ret = esp_bluedroid_init();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) return ret;
    ret = esp_ble_gap_register_callback(gap_cb);
    if (ret != ESP_OK) return ret;
    ret = esp_ble_gatts_register_callback(gatts_cb);
    if (ret != ESP_OK) return ret;
    return ESP_OK;
}

esp_err_t BLE_StartGATTServer(void)
{
    return esp_ble_gatts_app_register(BLE_APP_ID);
}

esp_err_t BLE_StartAdvertising(void)
{
    esp_ble_adv_data_t adv = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = true,
        .flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT,
    };
    return esp_ble_gap_config_adv_data(&adv);
}
