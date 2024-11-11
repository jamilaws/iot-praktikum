#pragma once


// #define EXAMPLE_ESP_WIFI_SSID      "CAPS-Seminar-Room"
// #define EXAMPLE_ESP_WIFI_PASS      "caps-schulz-seminar-room-wifi"
// #define SNTP_SERVER_NAME           "ntp1.in.tum.de"

#define EXAMPLE_ESP_WIFI_SSID      "FRITZ!Box 7430 TK"
#define EXAMPLE_ESP_WIFI_PASS      "87435946242696654701"
#define SNTP_SERVER_NAME           "pool.ntp.org"

// #define MQTT_BROKER                "131.159.85.91" // classroom

#define MQTT_BROKER                "192.168.178.63" // home

// for corridor
#define DEVICE_ID                  "2"
#define DEVICE_TOPIC               "1/2/data"
#define DEVICE_KEY                 "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3Mjk4NDQ5MjIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8yIn0.wF86Yw56TPwJG8OTvpGtGKZecU0ZipQbcr7QjgpZUIVC45pLepo2dO9Ad6IUB_w8g_ijvHOzlcgs1KlAv9ttrI3EDnuQrSBXhTJCLhK0kZHZwk1VKQxUJb4_UBlW1V9b7iCM8FK_aRfSleml_I5eFhqZIWAO5cnPEdCuPN0rJ0NsdhH047AgagAVvwIwZn5bViEjVuWGJZ6h2v-RGYaHWw_MYiGNbty04TpNWs2ZRPVxTCDJIkrO41iNbWjddrQOPFB4PhVCpfcZDzOnE5-lFAWZ6nptfI28C_JxIFkpOtiCHZLBRV8V-zl2_-GGn4fVuID6MZ8t1dVbxe_bQgqyBICdSa2oafzqhqtaXHUxlAL8omDPhIM_-2vKf6yGmb4LHqWrqMo_RrWPDBQb5JGmD1kk3qH7zTmkhzEB2oKPzxuvBvpgNYIOW7c6N1ooUNySphDy8Ry1rFNf8Xvmq2o_HmSuqdGQLuf25CJP8skdr4FooRzpkdZf-yHGPHPxH21donZ5TYYgKhfgnvQ5SZC-u3T-vlBizqOSjkyHhi-_gSnY2X9_Y7MeftPZJIHv0jw8YZXuje88vz2V1K1Hby1llZ7lEnldePgsTnGf8BweYxrR5Wq5dzX_ZlFN2sFQlrLMWlvshsdzLP9hQKNpwgOuu5IL857KkCUkRX58g6zuZXA"

// for bathroom
//#define DEVICE_ID                  "3"
//#define DEVICE_TOPIC               "1/3/data"
//#define DEVICE_KEY                 "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzA0OTAwNzIsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS8zIn0.gtQ1iVRv20Pz-YQmVfnGYAEYadGtQkAINfb9NKBt3IqGn0Tf_6NM3Idlvh9r5zh3Q-DSun393Q1qmQ7nSbvsMsA32KLcUI4RtjiL6Tbgbkc6pFbgk4Pie0J95lyLKjOuINit_wAIH8qK_jGRBA24guGdqd7AVERJzhvrJurOe2GtNENuT47BvkgnftCQx1W5M5SRq8QhW8rgRQTr7XNmTLHA2t1wxuTqGlzOLmKpzOHdYnucJzUvwKpZxaBex3ywqy9ZlJO4rmkJ9A5RlwG4791394tb6W-W0J77xFTUVWJ93waE2ro2q8Oyx_DxGZTKWq59L_3rJ-OMRafQfch73U5J7QKANfkq9dGGMSWSRnJFOQ3Nd0hRrWGhpUZTJSDdZfvYyThWFzZy91C00dWmN1K9cQ_Ld77gjReHB6t8OAKB4nPh6hHNOTyXWp48CcqZhsYl4uJQkLd2eanCEukRUn1-uug36ZDYsU8kfO84cNqILtz056FnDFWIAMforAjKRYIqJo-RcNn2kYwQeZq29NVqn5sdd-56n0aSk9GKUiR_nVP5u32BTHx2qmMkj-mmjs5BpOBbWls4Z8DBWI6o8D52a1gsDr_KIAhzMDqp1wtILgUTY-fpK7p-zaiNiGXZ6Rvcmm1CExwlLZyD0jgo2kBjraOYDPdC6kA8r4ds7vQ"

// for door
// #define DEVICE_ID                  "4"
// #define DEVICE_TOPIC               "1/4/data"
// #define DEVICE_KEY                 "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE3MzA2Mzg2MDQsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMS80In0.O9Uc5fiw12V46WMUnoFhi5bP-9jAOZVV6sFbm37t9paCo1rpYOiYOMnA4rqfJVW6FOlD3aZs_xP5WHFjJ_K-pesORigaHgw9NqN1K9gGM78GJV-dbki_wsyma_JyhFBPhuAa3fnH_ppqir1ev6X3auJJP-NdMcbAcjskX17TGp7XN3_6jkgR1ms2_2e_8FGbhtb_2d7EZ4xswzpIomj5QweW64hlJ7ExxJjP5pPVovfRMP5i2TR-ISmQ33cYh_cuABFzR5gnebkE52_HQHedK1JFjjcRSTfR7RI22XfKrPxi9dN3vsgjX9Iqs0kMcfWPzgFCrUWbHBMoVNrqed8-O8MJHAmMW8H7zzOlyadZSh9edCC5VDrR-kIzsARarVBbzhnh4IzSyS7GM1rjtmglpMTDyfXSgKPzHzUPUp9oSiNnmFcH67a0KTRQPNmfNxGYbZOGpKaXFbEYQl4dVoMEvhUb8S1a6g0GUu03qROfZx_g4tD6IT6LBOqNgDVLmNwM3ivqgIKWMmy3qg-aHoPl4Jn1ZfKOYu-aP1t3LkdM8Fbpi2XbtUMzR2aD2lmNuEWm6Dp5iB96gfqG-j0HWynRyw3UzZnANIyzqrYrFoxzOM2Qngjh15jIMQpVx_G5eX4gRoTdpqw2blS1zbTRSinmu978Q0EJHaWJ6tQpfF_7U8M"

#define PIR_PIN 27 
#define SWITCH_PIN 32
#define PIR_PIN2 23
#define PIR_PIN_MASK ((1ULL<<PIR_PIN) | (1ULL<<PIR_PIN2))
