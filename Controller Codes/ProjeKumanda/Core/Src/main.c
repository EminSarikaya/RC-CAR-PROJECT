/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "ssd1306_fonts.h"
#include "NRF24.h"
#include "NRF24_reg_addresses.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

//JOYSTICKLERDEN GELEN HAM VERİLERİN KAYDEDİLDİĞİ DİZİNİN KODLARI:
uint16_t adc_degerleri[5];

//JOYSTICKLERDEN GELEN HAM VERİLERİN İŞLENDİKTEN SONRAKİ DEĞİŞKENLERİNİN ADLARI:
int16_t ileriGeriDonusum;
int16_t sagSolDonusum;
int16_t onAmortisorDonusum;
int16_t arkaAmortisorDonusum;

//JOYSTICK BUTONLARININ HAFIZA DEĞİŞKENLERİ:
uint8_t solButonBaslangic = 1;
uint8_t sagButonBaslangic = 1;
uint8_t amortisorButonBaslangic = 1;

//JOYSTICK BUTONLARININ SONUÇ DEĞİŞKENLERİ:
uint8_t solButonGuncel = 0;
uint8_t sagButonGuncel = 0;
uint8_t amortisorButonGuncel = 0;

// 4x4 KEYPAD BASILAN TUŞUN KAYDEDİLDİĞİ DEĞİŞKEN:
uint8_t basilanTus = 0;
uint8_t basilanTusEski = 0;
uint8_t keypadBasiliButon[17] = {0};

//BATARYA VOLTAJININ ATANDIĞI DEĞİŞKEN:
float bataryaVoltaji = 0.0f;

//OLED EKRANIN ARAYÜZ KISMININ KODLARI:
uint8_t aktifSayfa = 1;
uint8_t islemBekleyenTus = 0;
uint8_t ayarModu = 0;

//OLED EKRAN İLK SAYFADAKİ DİNAMİK MESAJ DEĞİŞKENİ:
char altBilgiMesaji[25] = "SISTEM HAZIR";

//AYDINLATMA MENÜSÜ DEĞİŞKENLERİ:
uint8_t aydinlatmaDegerleri[9] = {100, 100, 100, 100, 100, 100, 100, 100, 100};
uint8_t aydinlatmaDurumu[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
char *aydinlatmaIsimleri[9] = {
    "1.On Farlar", "2.On Sisler", "3.Sag Sinyal",
    "4.Sol Sinyal", "5.Ic Ambiyans", "6.Bagaj Amb.",
    "7.Arka Stop", "8.Arac Alti", "9.Geri Vites"
};

uint8_t aydinlatmaSeciliSatir = 0;
uint8_t ekranUstLimit = 0;
uint8_t amerikanParkDurumu = 0;

//AYDINLATMA MENÜSÜ ÇAKAR MODU DEĞİŞKENLERİ:
uint8_t cakarModuAktif = 0;
uint32_t cakarZamanlayici = 0;
uint8_t cakarFlasor = 0;

//PARK SENSÖRÜ DEĞİŞKENİ:
uint8_t parkSensoru=0;

//NRF24L01 HABERLEŞME DEĞİŞKENLERİ:
typedef struct __attribute__((packed)) {
    uint8_t guvenlikKodu;      // 1 Byte
    int16_t ileriGeri;         // 2 Byte
    int16_t sagSol;            // 2 Byte
    int16_t onAmortisor;       // 2 Byte
    int16_t arkaAmortisor;     // 2 Byte
    uint8_t isikDurumlari[9];  // 9 Byte
    uint8_t isikParlaklik[9];  // 9 Byte
    uint8_t ekstraButonlar;    // 1 Byte
    uint8_t bosluk[4];         // 4 Byte
} KumandaVerisi;

KumandaVerisi gidenPaket;

//ARAÇTAN GELECEK OLAN TELEMETRİ VERİSİ KISMININ KODLARI:
typedef struct {
    float aracPili;
} TelemetriVerisi;

TelemetriVerisi gelenTelemetri;

uint8_t TxAddress[] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};

uint8_t sinyalSeviyesi = 4;
uint16_t kayipPaketSayaci = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
int32_t donusumFonksiyonu(int32_t anlikDeger, int32_t minimumGiris, int32_t maksimumGiris, int32_t minimumCikis, int32_t maksimumCikis);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_degerleri, 5);
  HAL_TIM_Base_Start_IT(&htim2);
  ssd1306_Init();

    //NRF24 VERİCİ (TX) AYARLARI;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(50);

    nrf24_init();
    nrf24_auto_ack_all(1);
    nrf24_auto_retr_limit(10);
    nrf24_auto_retr_delay(2);

    nrf24_tx_pwr(_0dbm);
    nrf24_data_rate(_1mbps);
    nrf24_set_channel(78);
    nrf24_set_crc(en_crc, _1byte);
    nrf24_pipe_pld_size(0, sizeof(KumandaVerisi));

    nrf24_open_tx_pipe(TxAddress);
    nrf24_open_rx_pipe(0, TxAddress);
    nrf24_stop_listen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  //TUŞ KONTROLLERİ VE MENÜ GEZİNTİSİ KISMININ KODU:
	  if(islemBekleyenTus != 0) {

	              //AYAR MODUNDA DEĞİŞKEN SAYFALAR ARASI GEÇİŞİ SAĞLAYAN KOD:
	              if(ayarModu == 0) {

	                  //SAYFALAR ARASI GEÇİŞ KODU:
	                  if(islemBekleyenTus == 12) {
	                      aktifSayfa++;
	                      if(aktifSayfa > 3) aktifSayfa = 1;
	                  }
	                  else if(islemBekleyenTus == 10) {
	                      aktifSayfa--;
	                      if(aktifSayfa < 1) aktifSayfa = 3;
	                  }

	                  //3.SAYFADAYKEN DEVREYE GİREN ALT-ÜST BUTON KISMININ KODU:
	                  else if(aktifSayfa == 3 && islemBekleyenTus == 7) {
	                      if(aydinlatmaSeciliSatir > 0) aydinlatmaSeciliSatir--;
	                      else aydinlatmaSeciliSatir = 8;
	                      if(aydinlatmaSeciliSatir < ekranUstLimit) ekranUstLimit = aydinlatmaSeciliSatir;
	                      else if(aydinlatmaSeciliSatir > ekranUstLimit + 3) ekranUstLimit = aydinlatmaSeciliSatir - 3;
	                  }
	                  else if(aktifSayfa == 3 && islemBekleyenTus == 15) {
	                      if(aydinlatmaSeciliSatir < 8) aydinlatmaSeciliSatir++;
	                      else aydinlatmaSeciliSatir = 0;
	                      if(aydinlatmaSeciliSatir > ekranUstLimit + 3) ekranUstLimit = aydinlatmaSeciliSatir - 3;
	                      else if(aydinlatmaSeciliSatir < ekranUstLimit) ekranUstLimit = aydinlatmaSeciliSatir;
	                  }
	                  else if(aktifSayfa == 3 && islemBekleyenTus == 11) {
	                      ayarModu = 1;
	                  }

	                  //4X4 KEYPAD TUŞLARININ İŞLEVLERİNİN ATANDIĞI KISMIN KODLARI:
	                  //TUŞ 1: ÖN FARLAR
	                  else if(islemBekleyenTus == 1) {
	                      aydinlatmaDurumu[0] = !aydinlatmaDurumu[0];
	                      sprintf(altBilgiMesaji, "FARLAR %s", aydinlatmaDurumu[0] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 2: SİSLER
	                  else if(islemBekleyenTus == 2) {
	                      aydinlatmaDurumu[1] = !aydinlatmaDurumu[1];
	                      sprintf(altBilgiMesaji, "SISLER %s", aydinlatmaDurumu[1] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 3: SAĞ SİNYAL
	                  else if(islemBekleyenTus == 3) {
	                      aydinlatmaDurumu[2] = !aydinlatmaDurumu[2];
	                      sprintf(altBilgiMesaji, "SAG SINYAL %s", aydinlatmaDurumu[2] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 4: SOL SİNYAL
	                  else if(islemBekleyenTus == 4) {
	                      aydinlatmaDurumu[3] = !aydinlatmaDurumu[3];
	                      sprintf(altBilgiMesaji, "SOL SINYAL %s", aydinlatmaDurumu[3] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 5: İÇ AMBİYANS
	                  else if(islemBekleyenTus == 5) {
	                      aydinlatmaDurumu[4] = !aydinlatmaDurumu[4];
	                      sprintf(altBilgiMesaji, "IC AMBIYANS %s", aydinlatmaDurumu[4] ? "ACIK" : "KPL");
	                  }
	                  //TUŞ 6: BAGAJ AMBİYANS
	                  else if(islemBekleyenTus == 6) {
	                      aydinlatmaDurumu[5] = !aydinlatmaDurumu[5];
	                      sprintf(altBilgiMesaji, "BAGAJ %s", aydinlatmaDurumu[5] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 8: ARKA STOP
	                  else if(islemBekleyenTus == 8) {
	                      aydinlatmaDurumu[6] = !aydinlatmaDurumu[6];
	                      sprintf(altBilgiMesaji, "STOPLAR %s", aydinlatmaDurumu[6] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 9: ARAÇ ALTI AYDINLATMA
	                  else if(islemBekleyenTus == 9) {
	                      aydinlatmaDurumu[7] = !aydinlatmaDurumu[7];
	                      sprintf(altBilgiMesaji, "ALT NEON %s", aydinlatmaDurumu[7] ? "ACIK" : "KAPALI");
	                  }
	                  //TUŞ 13: AMERİKAN PARK
	                  else if(islemBekleyenTus == 13) {
	                	  amerikanParkDurumu = !amerikanParkDurumu;
	                	      sprintf(altBilgiMesaji, "AMERIKAN PARK %s", amerikanParkDurumu ? "ACIK" : "KPL");
	                  }
	                  //TUŞ 14: ÇAKAR MODU
	                  else if(islemBekleyenTus == 14) {
	                      cakarModuAktif = !cakarModuAktif;
	                      if(cakarModuAktif) {
	                          sprintf(altBilgiMesaji, "CAKAR MODU AKTIF");
	                      } else {
	                          //ÇAKAR MODUNU KAPATINCA FAR-SİS KOMBİNASYONUNU KAPALI HALE GETİREN KOD:
	                          aydinlatmaDurumu[0] = 0;
	                          aydinlatmaDurumu[1] = 0;
	                          sprintf(altBilgiMesaji, "CAKAR KAPALI");
	                      }
	                  }
	                  else if(islemBekleyenTus == 16) {
	                	  parkSensoru = !parkSensoru;
	                	  sprintf(altBilgiMesaji, "PARK SENSORU %s", parkSensoru ? "ACIK" : "KPL");
	                  }

	              }
	              //AYAR MODUNA GİRİŞ YAPILDIĞINDA SADECE PARLAKLIK AYARININ YAPILABİLDİĞİ KISMIN KODU:
	              else if(ayarModu == 1) {
	                  //PARLAKLIĞI ARTIRAN KOD:
	            	  if(islemBekleyenTus == 12) {
	                      if(aydinlatmaDegerleri[aydinlatmaSeciliSatir] <= 90) {
	                          aydinlatmaDegerleri[aydinlatmaSeciliSatir] += 10;
	                      }
	                  }
	                  //PARLAKLIĞI AZALTAN KOD:
	            	  else if(islemBekleyenTus == 10) {
	                      if(aydinlatmaDegerleri[aydinlatmaSeciliSatir] >= 10) {
	                          aydinlatmaDegerleri[aydinlatmaSeciliSatir] -= 10;
	                      }
	                  }
	                  //AYAR MODUNDAN ÇIKAN TUŞUN KODU:
	            	  else if(islemBekleyenTus == 11) {
	                      ayarModu = 0;
	                  }
	              }

	              islemBekleyenTus = 0;

	        }
                  //GERİ VİTES LAMBASININ YANDIĞI KOD:
	              if(sagSolDonusum < -20) {
	                    aydinlatmaDurumu[8] = 1;
	                } else {
	                    aydinlatmaDurumu[8] = 0;
	                }

	                //ÇAKAR MODU KISMININ KODLARI:
	                if(cakarModuAktif == 1) {

	                    if(HAL_GetTick() - cakarZamanlayici > 70) {
	                        cakarZamanlayici = HAL_GetTick();
	                        cakarFlasor = !cakarFlasor;
                            //FARLARA VE SİSLERE TERLEME YAPTIRAN KISMIN KODU:
	                        aydinlatmaDurumu[0] = cakarFlasor;
	                        aydinlatmaDurumu[1] = !cakarFlasor;
	                    }
	                }


	        //OLED EKRAN ÇİZİMİ KODLARI:
	        ssd1306_Fill(Black);

	        	        //GÖNDERİLECEK VERİ PAKETİNİN HAZIRLANDIĞI KISMIN KODLARI:
	                    gidenPaket.guvenlikKodu = 0x55;
	                    gidenPaket.ileriGeri = ileriGeriDonusum;
	        	        gidenPaket.sagSol = sagSolDonusum;
	        	        gidenPaket.onAmortisor = onAmortisorDonusum;
	        	        gidenPaket.arkaAmortisor = arkaAmortisorDonusum;

	        	        for(int i = 0; i < 9; i++) {
	        	        	gidenPaket.isikDurumlari[i] = aydinlatmaDurumu[i];
	        	        	gidenPaket.isikParlaklik[i] = aydinlatmaDegerleri[i];
	        	        }

	        	        //JOYSTICKLERİN BUTONLARINI VE PARK SENSÖRÜ BUTONUNU TEK BİT İÇERİSİNE SIKIŞTIRMA KISMININ KODLARI:
	        	        gidenPaket.ekstraButonlar = (solButonGuncel) |
	        	        		(sagButonGuncel << 1) |
	        					(amortisorButonGuncel << 2) |
	        					(parkSensoru << 3) |
	        					(amerikanParkDurumu << 4);

	        	        //NRF24 HALF-DUPLEX İLETİŞİM GERÇEKLEŞTİĞİ KISMIN KODLARI:

	        	        //ÖNCE SÜRÜŞ VERİSİNİN GÖNDERİLDİĞİ KISMIN KODLARI:
	        	        nrf24_stop_listen();
	        	        nrf24_pipe_pld_size(0, sizeof(KumandaVerisi));
	        	        nrf24_open_tx_pipe(TxAddress);

	        	        uint8_t hamStatus = nrf24_transmit((uint8_t *)&gidenPaket, sizeof(KumandaVerisi));

	        	        if (hamStatus & (1 << 5)) {
	        	        	kayipPaketSayaci = 0;
	        	        	sinyalSeviyesi = 4;
	        	        } else {
	        	        	kayipPaketSayaci++;

	        	        	if(kayipPaketSayaci > 2)  sinyalSeviyesi = 3;
	        	        	if(kayipPaketSayaci > 5)  sinyalSeviyesi = 2;
	        	        	if(kayipPaketSayaci > 10) sinyalSeviyesi = 1;
	        	        	if(kayipPaketSayaci > 20) {
	        	        		sinyalSeviyesi = 0;
	        	        		if(kayipPaketSayaci > 1000) kayipPaketSayaci = 1000;
	        	        	}
	        	        }

	        	        //SONRASINDA HEMEN ARAÇTAN GELECEK PİL VERİSİNİN DİNLENDİĞİ KISMIN KODLARI:
	        	        nrf24_pipe_pld_size(0, sizeof(TelemetriVerisi));
	        	        nrf24_open_rx_pipe(0, TxAddress);
	        	        nrf24_listen();

	        	        //ARACIN VERİYİ İŞLEYİP GÖNDERMESİ İÇİN GEREKEN DİNLENME SÜRESİ:
	        	        HAL_Delay(10);

	        	        //ARAÇTAN GELEN PİL VERİSİNİN HAFIZAYA KAYDEDİLDİĞİ KISMIN KODU:
	        	        if (nrf24_data_available()) {
	        	        	nrf24_receive((uint8_t *)&gelenTelemetri, sizeof(TelemetriVerisi));
	        	        }

	        	        //OLED EKRAN ÇİZİMİNİN GERÇEKLEŞTİRİLDİĞİ KISMIN KODLARI:
	        	        ssd1306_Fill(Black);

	        	        switch(aktifSayfa) {
	        	        case 1:
	        	                      // --- SAYFA 1 TASARIMI ---
	        	                      ssd1306_SetCursor(1, 3);
	        	                      ssd1306_WriteString("4X4 BY EMIN DESIGN", Font_7x10, White);
	        	                      ssd1306_Line(0, 15, 128, 15, White);

	        	                      //KUMANDA PİL DEĞERLERİ:
	        	                      char voltajYazisi[20];
	        	                      sprintf(voltajYazisi, "K.Pil:%.1fV", bataryaVoltaji);
	        	                      ssd1306_SetCursor(48, 18);
	        	                      ssd1306_WriteString(voltajYazisi, Font_7x10, White);

	        	                      //ARAÇ PİL DEĞERLERİ:
	        	                      char aracPilMetni[15];
	        	                      if(sinyalSeviyesi == 0) {
	        	                    	  sprintf(aracPilMetni, "A.Pil: --- ");
	        	                      } else {
	        	                    	  sprintf(aracPilMetni, "A.Pil:%.1fV", gelenTelemetri.aracPili);
	        	                      }
	        	                      ssd1306_SetCursor(48, 34);
	        	                      ssd1306_WriteString(aracPilMetni, Font_7x10, White);

	        	                      //DİNAMİK SİNYAL ÇUBUKLARI KISMININ KODLARI:
	        	                      ssd1306_SetCursor(0, 18);
	        	                      ssd1306_WriteString("SINYAL", Font_7x10, White);

	        	                      if(sinyalSeviyesi > 0) ssd1306_DrawRectangle(2,  44, 6,  48, White);
	        	                      if(sinyalSeviyesi > 1) ssd1306_DrawRectangle(10, 39, 14, 48, White);
	        	                      if(sinyalSeviyesi > 2) ssd1306_DrawRectangle(18, 34, 22, 48, White);
	        	                      if(sinyalSeviyesi > 3) ssd1306_DrawRectangle(26, 29, 30, 48, White);

	        	                      if(sinyalSeviyesi == 0) {
	        	                    	  ssd1306_Line(2, 34, 30, 48, White);
	        	                    	  ssd1306_Line(30, 34, 2, 48, White);
	        	                      }

	        	                      ssd1306_Line(44, 15, 44, 50, White);

	        	                      //ALT BİLGİ METNİ KISMININ KODLARI:
	        	                      ssd1306_Line(0, 51, 128, 51, White);
	        	                      ssd1306_SetCursor(0, 54);
	        	                      ssd1306_WriteString(altBilgiMesaji, Font_7x10, White);
	        	                      break;

	        	        case 2:
	        	                      // --- SAYFA 2 TASARIMI ---
	        	                      ssd1306_SetCursor(11, 3);
	        	                      ssd1306_WriteString("JOYSTICK VERISI", Font_7x10, White);
	        	                      ssd1306_Line(0, 15, 128, 15, White);

	        	                      int16_t barlar[4] = {sagSolDonusum, ileriGeriDonusum, onAmortisorDonusum, arkaAmortisorDonusum};
	        	                      char *isimler[4] = {"I/G", "S/S", "O.A", "A.A"};

	        	                      for(int i = 0; i < 4; i++) {
	        	                          int y = 18 + (i * 11);
	        	                          ssd1306_SetCursor(0, y);
	        	                          ssd1306_WriteString(isimler[i], Font_7x10, White);

	        	                          ssd1306_Line(74, y, 74, y+8, White);
	        	                          ssd1306_Line(75, y, 75, y+8, White);

	        	                          int val = barlar[i];
	        	                          int genislik = (val < 0 ? -val : val) * 50 / 255;

	        	                          if(val > 0) {
	        	                              for(int j = 0; j < 6; j++) {
	        	                                  ssd1306_Line(77, y + 1 + j, 77 + genislik, y + 1 + j, White);
	        	                              }
	        	                          }
	        	                          else if(val < 0) {
	        	                              for(int j = 0; j < 6; j++) {
	        	                                  ssd1306_Line(72 - genislik, y + 1 + j, 72, y + 1 + j, White);
	        	                              }
	        	                          }
	        	                      }
	        	                      break;

	        	        case 3:
	        	                      // --- SAYFA 3 TASARIMI ---
	        	                      ssd1306_SetCursor(12, 3);
	        	                      ssd1306_WriteString("AYDINLATMA MODU", Font_7x10, White);
	        	                      ssd1306_Line(0, 15, 128, 15, White);

	        	                      for(int i = 0; i < 4; i++) {
	        	                          int satirIndex = ekranUstLimit + i;
	        	                          if(satirIndex > 8) break;

	        	                          int y_kordinat = 18 + (i * 11);

	        	                          if(satirIndex == aydinlatmaSeciliSatir) {
	        	                              ssd1306_SetCursor(0, y_kordinat);
	        	                              if(ayarModu == 1) ssd1306_WriteString("*", Font_7x10, White);
	        	                              else ssd1306_WriteString(">", Font_7x10, White);
	        	                          }

	        	                          ssd1306_SetCursor(10, y_kordinat);
	        	                          ssd1306_WriteString(aydinlatmaIsimleri[satirIndex], Font_7x10, White);

	        	                          char parlaklikYazisi[15];
	        	                          if(aydinlatmaDurumu[satirIndex] == 1) {
	        	                              sprintf(parlaklikYazisi, "[A]%%%d", aydinlatmaDegerleri[satirIndex]);
	        	                          } else {
	        	                              sprintf(parlaklikYazisi, "[K]%%%d", aydinlatmaDegerleri[satirIndex]);
	        	                          }

	        	                          ssd1306_SetCursor(82, y_kordinat);
	        	                          ssd1306_WriteString(parlaklikYazisi, Font_7x10, White);
	        	                      }
	        	                      break;
	        	        }

	        	        ssd1306_UpdateScreen();
	        	        HAL_Delay(40);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 199;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB12 PB13 PB14
                           PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB3 PB4
                           PB5 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//HAM OLARAK ALINAN DEĞERLERİN İŞLENDİĞİ FONKSİYONUN KODLARI:
int32_t donusumFonksiyonu(int32_t anlikDeger, int32_t minimumGiris, int32_t maksimumGiris, int32_t minimumCikis, int32_t maksimumCikis){
	return (anlikDeger - minimumGiris) * (maksimumCikis - minimumCikis) / (maksimumGiris - minimumGiris) + minimumCikis;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
    	//HAM DEĞERLERİN İŞLENDİĞİ KISMIN KODLARI:
        ileriGeriDonusum = donusumFonksiyonu(adc_degerleri[0], 50, 4045, -255, 255);
        sagSolDonusum = donusumFonksiyonu(adc_degerleri[1], 50, 4045, -255, 255);
        onAmortisorDonusum = donusumFonksiyonu(adc_degerleri[2], 50, 4045, -255, 255);
        arkaAmortisorDonusum = donusumFonksiyonu(adc_degerleri[3], 50, 4045, -255, 255);
        bataryaVoltaji = (adc_degerleri[4] * 3.3f * (14.7f / 4.7f)) / 4095.0f;

        //ÜST ÖLÜ BÖLGENİN SIFIRLANDIĞI KISMIN KODLARI:
        if(ileriGeriDonusum > 255) {
            ileriGeriDonusum = 255;
        }
        else if(ileriGeriDonusum < -255) {
            ileriGeriDonusum = -255;
        }

        if(sagSolDonusum > 255) {
            sagSolDonusum = 255;
        }
        else if(sagSolDonusum < -255) {
            sagSolDonusum = -255;
        }

        if(onAmortisorDonusum > 255) {
            onAmortisorDonusum = 255;
        }
        else if(onAmortisorDonusum < -255) {
            onAmortisorDonusum = -255;
        }

        if(arkaAmortisorDonusum > 255) {
            arkaAmortisorDonusum = 255;
        }
        else if(arkaAmortisorDonusum < -255) {
            arkaAmortisorDonusum = -255;
        }

        //ALT ÖLÜ BÖLGENİN SIFIRLANDIĞI KISMIN KODLARI:
        if(ileriGeriDonusum > -20 && ileriGeriDonusum < 20) {
        	ileriGeriDonusum = 0;
        }
        if(sagSolDonusum > -20 && sagSolDonusum < 20) {
        	sagSolDonusum = 0;
        }
        if(onAmortisorDonusum > -20 && onAmortisorDonusum < 20) {
        	onAmortisorDonusum = 0;
        }
        if(arkaAmortisorDonusum > -20 && arkaAmortisorDonusum < 20) {
        	arkaAmortisorDonusum = 0;
        }

        //JOYSTICK BUTONLARININ OKUNDUĞU KISMIN KODLARI:
        uint8_t anlikOkuma;

        anlikOkuma = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
        if(anlikOkuma == 0 && solButonBaslangic == 1) {
            solButonGuncel = !solButonGuncel;
        }
        solButonBaslangic = anlikOkuma;

        anlikOkuma = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);
        if(anlikOkuma == 0 && sagButonBaslangic == 1) {
            sagButonGuncel = !sagButonGuncel;
        }
        sagButonBaslangic = anlikOkuma;

        anlikOkuma = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
        if(anlikOkuma == 0 && amortisorButonBaslangic == 1) {
            amortisorButonGuncel = !amortisorButonGuncel;
        }
        amortisorButonBaslangic = anlikOkuma;

        //4X4 KEYPAD TUŞLARININ TARAMA YÖNTEMİYLE TESPİT EDİLDİĞİ KISMIN KODLARI:
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);

                basilanTus = 0;

                //1.SATIRIN TARANDIĞI KISMIN KODLARI:
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0)  basilanTus = 16;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == 0)  basilanTus = 15;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0) basilanTus = 14;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0) basilanTus = 13;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

                //2.SATIRIN TARANDIĞI KISMIN KODLARI:
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0)  basilanTus = 12;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == 0)  basilanTus = 11;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0) basilanTus = 10;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0) basilanTus = 9;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);

                //3.SATIRIN TARANDIĞI KISMIN KODLARI:
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0)  basilanTus = 8;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == 0)  basilanTus = 7;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0) basilanTus = 6;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0) basilanTus = 5;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

                //4.SATIRIN TARANDIĞI KISMIN KODLARI:
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0)  basilanTus = 4;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9) == 0)  basilanTus = 3;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0) basilanTus = 2;
                if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0) basilanTus = 1;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);

                //TUŞ DEĞERLERİNİN DEĞİŞKENE KAYDEDİLDİĞİ KISMIN KODU:
                if(basilanTus != 0 && basilanTusEski == 0) {
                	keypadBasiliButon[basilanTus] = !keypadBasiliButon[basilanTus];

                	islemBekleyenTus = basilanTus;
                }

                basilanTusEski = basilanTus;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
