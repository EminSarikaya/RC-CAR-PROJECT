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
#include <stdlib.h>
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

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */

//KUMANDA İLE AYNI PAKET YAPISINA SAHİP STRUCT KISMININ KODU:
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
} KumandaVerisi;               //TOPLAM 32 Byte

//KUMANDADAN GELEN VERİLERİN KISMININ KODLARI:
KumandaVerisi gelenPaket;
uint8_t RxAddress[] = { 0xEE, 0xDD, 0xCC, 0xBB, 0xAA };
uint32_t sonSinyalZamani = 0;

//SÜSPANSİYON HAFIZA DEĞİŞKENLERİ:
int pwmOnHafiza = 150;
int pwmArkaHafiza = 150;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_IWDG_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	MX_SPI1_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_IWDG_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */
	__HAL_AFIO_REMAP_SWJ_NOJTAG();

	//VERİ AKTARIMINDAN EMİN OLMAK İÇİN PC13 LEDİ BAŞLANGIÇTA SÖNDÜRÜLÜR:
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

	//MOTORLAR İÇİN PWM'NİN BAŞLADIĞI KISMIN KODU:
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

	HAL_TIM_Base_Start_IT(&htim4);

	//NRF24 ALICI RX YAPILANDIRMALARININ YAPILDIĞI KISMIN KODU:
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_Delay(50);

	nrf24_init();
	nrf24_tx_pwr(_0dbm);
	nrf24_data_rate(_1mbps);
	nrf24_set_channel(78);
	nrf24_set_crc(en_crc, _1byte);
	nrf24_pipe_pld_size(0, sizeof(KumandaVerisi));
	nrf24_auto_ack_all(1);

	nrf24_open_rx_pipe(0, RxAddress);
	nrf24_listen();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */

		if (nrf24_data_available()) {

			//NRF24'ÜN ANLIK DURUMUNU KONTROL EDEREK KİTLENME DURUMLARINDA KORUMA MODUNA GEÇİRMEYE YARAYAN DEĞİŞKEN:
			uint8_t nrfDurum = nrf24_r_status();

			//ÇİPİN MANTIKLI DEĞERLER ÜRETTİĞİ ZAMAN VERİLERİN İŞLENDİĞİ KISMIN KODLARI:
			if (nrfDurum != 0x00 && nrfDurum != 0xFF) {

				nrf24_receive((uint8_t*) &gelenPaket, sizeof(KumandaVerisi));

				//GELEN PAKETİN GÜVENLİK KODU 0x55 İSE İŞLEME GİREN KISMIN KODLARI:
				if (gelenPaket.guvenlikKodu == 0x55) {

					sonSinyalZamani = HAL_GetTick();
					HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

					//DİFERANSİYEL SÜRÜŞ HESAPLAMASI (TANK MODU) KISMININ KODLARI:
					int16_t solMotorHiz = gelenPaket.ileriGeri
							+ gelenPaket.sagSol;
					int16_t sagMotorHiz = gelenPaket.ileriGeri
							- gelenPaket.sagSol;

					//MOTOR UYANMA EŞİĞİ VE TEMBEL MOTOR KALİBRASYONU KISMININ KODLARI:
					int16_t uyanmaEsigi = 50;

					// SOL MOTOR İÇİN MİNİMUM GÜÇ BAŞLATMA NOKTASU:
					if(solMotorHiz > 0) solMotorHiz += uyanmaEsigi;
					else if(solMotorHiz < 0) solMotorHiz -= uyanmaEsigi;

					// SAĞ MOTOR İÇİN MİNİMUM GÜÇ BAŞLATMA NOKTASI:
					if(sagMotorHiz > 0) sagMotorHiz += uyanmaEsigi;
					else if(sagMotorHiz < 0) sagMotorHiz -= uyanmaEsigi;


					//HIZ DEĞERLERİNİN PWM SINIRLARINA GÖRE AYARLANDIĞI KISMIN KODLARI:
					int16_t hizSiniri=220;

					if (solMotorHiz > hizSiniri) {
						solMotorHiz = hizSiniri;
					}
					if (solMotorHiz < -hizSiniri) {
						solMotorHiz = -hizSiniri;
					}
					if (sagMotorHiz > hizSiniri) {
						sagMotorHiz = hizSiniri;
					}
					if (sagMotorHiz < -hizSiniri) {
						sagMotorHiz = -hizSiniri;
					}

					//SOL MOTORUN YÖN VE GÜÇ KONTROLÜ KISMININ KODLARI:
					if (solMotorHiz > 0) {
						HAL_GPIO_WritePin(GPIOB, MOTOR_SOL_ILERI_Pin,
								GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, MOTOR_SOL_GERI_Pin,
								GPIO_PIN_RESET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
								solMotorHiz);
					} else if (solMotorHiz < 0) {
						HAL_GPIO_WritePin(GPIOB, MOTOR_SOL_ILERI_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, MOTOR_SOL_GERI_Pin,
								GPIO_PIN_SET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
								-solMotorHiz);
					} else {
						HAL_GPIO_WritePin(GPIOB, MOTOR_SOL_ILERI_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, MOTOR_SOL_GERI_Pin,
								GPIO_PIN_RESET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
					}

					//SAĞ MOTORUN YÖN VE GÜÇ KONTROLÜ KISMININ KODLARI:
					if (sagMotorHiz > 0) {
						HAL_GPIO_WritePin(GPIOB, MOTOR_SAG_ILERI_Pin,
								GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB, MOTOR_SAG_GERI_Pin,
								GPIO_PIN_RESET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
								sagMotorHiz);
					} else if (sagMotorHiz < 0) {
						HAL_GPIO_WritePin(GPIOB, MOTOR_SAG_ILERI_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, MOTOR_SAG_GERI_Pin,
								GPIO_PIN_SET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
								-sagMotorHiz);
					} else {
						HAL_GPIO_WritePin(GPIOB, MOTOR_SAG_ILERI_Pin,
								GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB, MOTOR_SAG_GERI_Pin,
								GPIO_PIN_RESET);
						__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
					}
				}
			}
		}

		//KUMANDADAN VERİ GELMEDİĞİNDE ARACIN KORUMA MODUNA ALINDIĞI KISMIN KODLARI:
		if ((HAL_GetTick() - sonSinyalZamani) > 300) {
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
			HAL_GPIO_WritePin(GPIOB,
					MOTOR_SOL_ILERI_Pin | MOTOR_SOL_GERI_Pin
							| MOTOR_SAG_ILERI_Pin | MOTOR_SAG_GERI_Pin,
					GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

			//BAĞLANTI KOPARSA IŞIKLARIN SÖNDÜRÜLDÜĞÜ KISMIN KODLARI:
			for (int i = 0; i < 9; i++) {
				gelenPaket.isikDurumlari[i] = 0;
			}

			//SERVOLARIN KORUMA MODUNA ALINDIĞI KISMIN KODLARI:

			pwmOnHafiza = 150;
			pwmArkaHafiza = 150;
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwmOnHafiza);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwmArkaHafiza);

		}

		//AMORTİSÖR / SERVO KONTROLLERİNİN YAPILDIĞI KISMIN KODU:

		//GÜVENLİK AMAÇLI AMORTİSÖRLERE KİLİT MANTIĞININ UYGULANDIĞI KISMIN KODLARI:
		uint8_t amortisorKilitAcik = (gelenPaket.ekstraButonlar & (1 << 2));
        uint8_t amortisorAciSiniri = 20;
		if (amortisorKilitAcik) {

			// 150 MERKEZ NOKTASINA +/- 33 BİRİM EKLENEREK +/- 60 DERECE AÇI ELDE EDİLEN KISMIN KODLARI:
			pwmOnHafiza = 150 + ((gelenPaket.onAmortisor * amortisorAciSiniri) / 255);
			pwmArkaHafiza = 150 + ((gelenPaket.arkaAmortisor * amortisorAciSiniri) / 255);

			//+/- 60 DERECENİN DIŞINA ÇIKILMASININ ENGELLENDİĞİ KISMIN KODLARI:
			if (pwmOnHafiza > 150+amortisorAciSiniri)
				pwmOnHafiza = 150+amortisorAciSiniri;
			if (pwmOnHafiza < 150-amortisorAciSiniri)
				pwmOnHafiza = 150-amortisorAciSiniri;
			if (pwmArkaHafiza > 150+amortisorAciSiniri)
				pwmArkaHafiza = 150+amortisorAciSiniri;
			if (pwmArkaHafiza < 150-amortisorAciSiniri)
				pwmArkaHafiza = 150-amortisorAciSiniri;
		}

		//HAFIZADAKİ DEĞERLERİN SERVOYA GÖNDERİLDİĞİ KISMIN KODLARI:
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwmOnHafiza);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwmArkaHafiza);

		HAL_Delay(10);
		//SİSTEM KİLİTLENDİĞİ VAKİT ARACIN GÜCÜNÜ KESECEK OLAN KISMIN KODU:
		HAL_IWDG_Refresh(&hiwdg);
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_9;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void) {

	/* USER CODE BEGIN IWDG_Init 0 */

	/* USER CODE END IWDG_Init 0 */

	/* USER CODE BEGIN IWDG_Init 1 */

	/* USER CODE END IWDG_Init 1 */
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
	hiwdg.Init.Reload = 1250;
	if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN IWDG_Init 2 */

	/* USER CODE END IWDG_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

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
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
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
static void MX_TIM1_Init(void) {

	/* USER CODE BEGIN TIM1_Init 0 */

	/* USER CODE END TIM1_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };
	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

	/* USER CODE BEGIN TIM1_Init 1 */

	/* USER CODE END TIM1_Init 1 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 71;
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 255;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM1_Init 2 */

	/* USER CODE END TIM1_Init 2 */
	HAL_TIM_MspPostInit(&htim1);

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 719;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 1999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
	HAL_TIM_MspPostInit(&htim2);

}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 71;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65535;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */

}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void) {

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 71;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 39;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */

	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, SAG_SINYAL_Pin | SOL_SINYAL_Pin | ON_FAR_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
			GPIO_PIN_0 | GERI_VITES_Pin | ON_SIS_Pin | MOTOR_SOL_ILERI_Pin
					| MOTOR_SOL_GERI_Pin | MOTOR_SAG_ILERI_Pin
					| MOTOR_SAG_GERI_Pin | ARKA_STOP_Pin | IC_AMBIYANS_Pin
					| BAGAJ_AMBIYANS_Pin | ARAC_ALTI_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : SAG_SINYAL_Pin SOL_SINYAL_Pin PA4 ON_FAR_Pin */
	GPIO_InitStruct.Pin = SAG_SINYAL_Pin | SOL_SINYAL_Pin | GPIO_PIN_4
			| ON_FAR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pins : PB0 GERI_VITES_Pin ON_SIS_Pin MOTOR_SOL_ILERI_Pin
	 MOTOR_SOL_GERI_Pin MOTOR_SAG_ILERI_Pin MOTOR_SAG_GERI_Pin ARKA_STOP_Pin
	 IC_AMBIYANS_Pin BAGAJ_AMBIYANS_Pin ARAC_ALTI_Pin */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GERI_VITES_Pin | ON_SIS_Pin
			| MOTOR_SOL_ILERI_Pin | MOTOR_SOL_GERI_Pin | MOTOR_SAG_ILERI_Pin
			| MOTOR_SAG_GERI_Pin | ARKA_STOP_Pin | IC_AMBIYANS_Pin
			| BAGAJ_AMBIYANS_Pin | ARAC_ALTI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */

	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM4) {
		static uint8_t pwmSayaci = 0;
		pwmSayaci++;

		//SİNYAL BLİNK PERİYODUNUN BELİRLENDİĞİ KISIM:
		uint8_t sinyalBlink = (HAL_GetTick() / 400) % 2;

		//KUMANDADAN GELEN AMERİKAN PARK BİLGİSİ KISMI:
		uint8_t amerikanParkAcik = (gelenPaket.ekstraButonlar & (1 << 4));

		//ÖN FAR KISMININ KODU:

		if(gelenPaket.isikDurumlari[0] && (pwmSayaci < gelenPaket.isikParlaklik[0]))
			HAL_GPIO_WritePin(ON_FAR_GPIO_Port, ON_FAR_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(ON_FAR_GPIO_Port, ON_FAR_Pin, GPIO_PIN_RESET);

		//ÖN SİS KISMININ KODU:
		if (gelenPaket.isikDurumlari[1]
				&& (pwmSayaci < gelenPaket.isikParlaklik[1]))
			HAL_GPIO_WritePin(GPIOB, ON_SIS_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOB, ON_SIS_Pin, GPIO_PIN_RESET);

		//SAĞ SİNYAL KISMININ KODU:
		uint8_t sagLedDurumu = 0;
		if (gelenPaket.isikDurumlari[2]) {
			sagLedDurumu = sinyalBlink;
		}                          // Sinyal butonu açıksa amerikan parklara nazaran önceliklidir.
		else if (amerikanParkAcik)
			sagLedDurumu = 1;      // Sinyal yok ama amerikan parklar açıksa sabit yanar.

		if (sagLedDurumu && (pwmSayaci < gelenPaket.isikParlaklik[2]))
			HAL_GPIO_WritePin(GPIOA, SAG_SINYAL_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOA, SAG_SINYAL_Pin, GPIO_PIN_RESET);

		//SOL SİNYAL KISMININ KODU:
		uint8_t solLedDurumu = 0;
		if (gelenPaket.isikDurumlari[3]) {
			solLedDurumu = sinyalBlink;
		}                          // Sinyal butonu açıksa amerikan parklara nazaran önceliklidir.
		else if (amerikanParkAcik)
			solLedDurumu = 1;      // Sinyal yok ama amerikan parklar açıksa sabit yanar.

		if (solLedDurumu && (pwmSayaci < gelenPaket.isikParlaklik[3]))
			HAL_GPIO_WritePin(GPIOA, SOL_SINYAL_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOA, SOL_SINYAL_Pin, GPIO_PIN_RESET);

		//İÇ AMBİYANS KISMININ KODU:
		if (gelenPaket.isikDurumlari[4]
				&& (pwmSayaci < gelenPaket.isikParlaklik[4]))
			HAL_GPIO_WritePin(GPIOB, IC_AMBIYANS_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOB, IC_AMBIYANS_Pin, GPIO_PIN_RESET);

		//BAGAJ AMBİYANS KISMININ KODU:
		if (gelenPaket.isikDurumlari[5]
				&& (pwmSayaci < gelenPaket.isikParlaklik[5]))
			HAL_GPIO_WritePin(GPIOB, BAGAJ_AMBIYANS_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOB, BAGAJ_AMBIYANS_Pin, GPIO_PIN_RESET);

		//ARKA STOP KISMININ KODU:
		if (gelenPaket.isikDurumlari[6]
				&& (pwmSayaci < gelenPaket.isikParlaklik[6]))
			HAL_GPIO_WritePin(GPIOB, ARKA_STOP_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOB, ARKA_STOP_Pin, GPIO_PIN_RESET);

		//ARAÇ ALTI AYDINLATMA KISMININ KODU:
		if (gelenPaket.isikDurumlari[7]
				&& (pwmSayaci < gelenPaket.isikParlaklik[7]))
			HAL_GPIO_WritePin(GPIOB, ARAC_ALTI_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOB, ARAC_ALTI_Pin, GPIO_PIN_RESET);

		//GERİ VİTES KISMININ KODU:
		if (gelenPaket.isikDurumlari[8]
				&& (pwmSayaci < gelenPaket.isikParlaklik[8]))
			HAL_GPIO_WritePin(GPIOB, GERI_VITES_Pin, GPIO_PIN_SET);
		else
			HAL_GPIO_WritePin(GPIOB, GERI_VITES_Pin, GPIO_PIN_RESET);
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
