/**

******************************************************************************
  * @file    GPIO/GPIO_EXTI/Src/main.c
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    21-April-2017
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32L4xx HAL API.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************


this program: 

1. This project needs the libraray file i2c_at2464c.c and its header file. 
2. in the i2c_at2464c.c, the I2C SCL and SDA pins are configured as PULLUP. so do not need to pull up resistors (even do not need the 100 ohm resisters).
NOTE: students can also configure the TimeStamp pin 	

*/




/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_EXTI
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef  pI2c_Handle;

RTC_HandleTypeDef RTCHandle;
RTC_DateTypeDef RTC_DateStructure;
RTC_TimeTypeDef RTC_TimeStructure;

__IO HAL_StatusTypeDef Hal_status;  //HAL_ERROR, HAL_TIMEOUT, HAL_OK, of HAL_BUSY 

//memory location to write to in the device
__IO uint16_t Memory_Space = 0x000A; //pick any location within range

int mode;//used to change mode before enter the modification parts
char lcd_buffer[6];    // LCD display buffer
uint8_t ShowTemp[10]={0};  //array used to store time
uint8_t ShowTemp1[20]={0};  //array used to store time
uint8_t ShowTemp2[20]={0};  //array used to store time
char datestring[20]={0};	//array used to store date
uint8_t tempData[6];//used to store data
uint8_t SP;
int modify_mode;//used to change mode in the modification parts
uint8_t state = 0;
const char * Weekday[8] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};//array stores weekday strings
int WDN;//int to indicate index of weekday array

uint8_t wd, dd, mo, yy, ss, mm, hh; // for weekday, day, month, year, second, minute, hour

__IO uint32_t SEL_Pressed_StartTick;   //sysTick when the User button is pressed

__IO uint8_t leftpressed, rightpressed, uppressed, downpressed, selpressed;  // button pressed 
__IO uint8_t  sel_held;   // if the selection button is held for a while (>800ms)

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

void RTC_Config(void);
void RTC_AlarmAConfig(void);
void RTC_Config(void);
void RTC_AlarmAConfig(void);
void RTC_TimeShow(void);
void RTC_DateDisplay(void);
void RTC_SavePressTime(void);
void EEPROM_SaveTime(uint8_t hour, uint8_t min, uint8_t sec);
void EEPROM_ReadTime(uint8_t SPoffset);
void Pushbutton_Init(void);
void I2C_Store_time(void);
void Display_Last2times();

#define RTC_ASYNCH_PREDIV  127
#define RTC_SYNCH_PREDIV   249

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32L4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4 
       - Low Level Initialization
     */

	leftpressed=0;
	rightpressed=0;
	uppressed=0;
	downpressed=0;
	selpressed=0;
	sel_held=0;
	//WDN = 1;
	SP = 0;
	mode = 0;
	modify_mode = 1;
	//var initialization finished
	HAL_Init();
	
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
  
	SystemClock_Config();   
											
	
	HAL_InitTick(0x0000); //set the systick interrupt priority to the highest, !!!This line need to be after systemClock_config()

	
	BSP_LCD_GLASS_Init();
	
	BSP_JOY_Init(JOY_MODE_EXTI);

	BSP_LCD_GLASS_DisplayString((uint8_t*)"MT3TA4");	
	HAL_Delay(1000);


//configure real-time clock
	RTC_Config();
	
	RTC_AlarmAConfig();

	I2C_Init(&pI2c_Handle);


/*********************Testing I2C EEPROM------------------

	//the following variables are for testging I2C_EEPROM
	uint8_t data1 =0x67,  data2=0x68;
	uint8_t readData=0x00;
	uint16_t EE_status;

	EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space, data1);

  
  if(EE_status != HAL_OK)
  {
    I2C_Error(&pI2c_Handle);
  }
	*/
	
	/*
	BSP_LCD_GLASS_Clear();
	if (EE_status==HAL_OK) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 1 ok");
	}else
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 1 X");

	HAL_Delay(1000);
	
	EE_status=I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space+1 , data2);
	
  if(EE_status != HAL_OK)
  {
    I2C_Error(&pI2c_Handle);
  }
	
	BSP_LCD_GLASS_Clear();
	if (EE_status==HAL_OK) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 2 ok");
	}else
			BSP_LCD_GLASS_DisplayString((uint8_t*)"w 2 X");

	HAL_Delay(1000);
	
	readData=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space);

	BSP_LCD_GLASS_Clear();
	if (data1 == readData) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"r 1 ok");;
	}else{
			BSP_LCD_GLASS_DisplayString((uint8_t*)"r 1 X");
	}	
	
	HAL_Delay(1000);
	
	readData=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space+1);

	BSP_LCD_GLASS_Clear();
	if (data2 == readData) {
			BSP_LCD_GLASS_DisplayString((uint8_t*)"r 2 ok");;
	}else{
			BSP_LCD_GLASS_DisplayString((uint8_t *)"r 2 X");
	}	

	HAL_Delay(1000);
	*/


//******************************testing I2C EEPROM*****************************	
		

  /* Infinite loop */
  while (1)
  {
			//the joystick is pulled down. so the default status of the joystick is 0, when pressed, get status of 1. 
			//while the interrupt is configured at the falling edge---the moment the pressing is released, the interrupt is triggered.
			//therefore, the variable "selpressed==1" can not be used to make choice here.
			if (BSP_JOY_GetState() == JOY_SEL) {
					SEL_Pressed_StartTick=HAL_GetTick(); 
					while(BSP_JOY_GetState() == JOY_SEL) {  //while the selection button is pressed)	
						if ((HAL_GetTick()-SEL_Pressed_StartTick)<800) {	
							selpressed++;
							I2C_Store_time();
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t *)"TSTORED");	
							if (selpressed==1){
								for(int i = 0;i < 3;i++){
									tempData[i] = I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space-1-i-3);//save data in I2C to tempData
								}			
							}
							else if (selpressed==2){
								for(int i = 3;i < 6;i++){
									tempData[i] = I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space-1-i-3);//save data in I2C to tempData
								}		
							selpressed = 0;
							}
						} 
						else{
							RTC_AlarmA_IT_Disable(&RTCHandle);
							BSP_LCD_GLASS_Clear();
							RTC_DateDisplay();
							RTC_AlarmA_IT_Enable(&RTCHandle);
						}
					}
			}
	}
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = MSI
  *            SYSCLK(Hz)                     = 4000000
  *            HCLK(Hz)                       = 4000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            Flash Latency(WS)              = 0
  * @param  None
  * @retval None
  */



void SystemClock_Config(void)
{ 
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};                                            

  // RTC requires to use HSE (or LSE or LSI, suspect these two are not available)
	//reading from RTC requires the APB clock is 7 times faster than HSE clock, 
	//so turn PLL on and use PLL as clock source to sysclk (so to APB)
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;     //RTC need either HSE, LSE or LSI           
  
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;  
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6; // RCC_MSIRANGE_6 is for 4Mhz. _7 is for 8 Mhz, _9 is for 16..., _10 is for 24 Mhz, _11 for 48Hhz
  RCC_OscInitStruct.MSICalibrationValue= RCC_MSICALIBRATION_DEFAULT;
  
	//RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;//RCC_PLL_NONE;

	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;   //PLL source: either MSI, or HSI or HSE, but can not make HSE work.
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40; 
  RCC_OscInitStruct.PLL.PLLR = 2;  //2,4,6 or 8
  RCC_OscInitStruct.PLL.PLLP = 7;   // or 17.
  RCC_OscInitStruct.PLL.PLLQ = 4;   //2, 4,6, 0r 8  
	//the PLL will be MSI (4Mhz)*N /M/R = 

	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    // Initialization Error 
    while(1);
  }

  // configure the HCLK, PCLK1 and PCLK2 clocks dividers 
  // Set 0 Wait State flash latency for 4Mhz 
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; //the freq of pllclk is MSI (4Mhz)*N /M/R = 80Mhz 
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  
	
	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)   //???
  {
    // Initialization Error 
    while(1);
  }

  // The voltage scaling allows optimizing the power consumption when the device is
  //   clocked below the maximum system frequency, to update the voltage scaling value
  //   regarding system frequency refer to product datasheet.  

  // Enable Power Control clock 
  __HAL_RCC_PWR_CLK_ENABLE();

  if(HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    // Initialization Error 
    while(1);
  }

  // Disable Power Control clock   //why disable it?
  __HAL_RCC_PWR_CLK_DISABLE();      
}
//after RCC configuration, for timmer 2---7, which are one APB1, the TIMxCLK from RCC is 4MHz


void RTC_Config(void) {
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	

	//****1:***** Enable the RTC domain access (enable wirte access to the RTC )
			//1.1: Enable the Power Controller (PWR) APB1 interface clock:
        __HAL_RCC_PWR_CLK_ENABLE();    
			//1.2:  Enable access to RTC domain 
				HAL_PWR_EnableBkUpAccess();    
			//1.3: Select the RTC clock source
				__HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE);    
				//RCC_RTCCLKSOURCE_LSI is defined in hal_rcc.h
	       // according to P9 of AN3371 Application Note, LSI's accuracy is not suitable for RTC application!!!! 
				
			//1.4: Enable RTC Clock
			__HAL_RCC_RTC_ENABLE();   //enable RTC --see note for the Macro in _hal_rcc.h---using this Marco requires 
																//the above three lines.
			
	
			//1.5  Enable LSI
			__HAL_RCC_LSI_ENABLE();   //need to enable the LSI !!!
																//defined in _rcc.c
			while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)==RESET) {}    //defind in rcc.c
	
			// for the above steps, please see the CubeHal UM1725, p616, section "Backup Domain Access" 	
				
				
				
	//****2.*****  Configure the RTC Prescaler (Asynchronous and Synchronous) and RTC hour 
        
		
		/************students: need to complete the following lines******************************
		**************************************************************************************/				
				RTCHandle.Instance = RTC;
				RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
				
				RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV; 
				RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV; 
				
				
				RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
				RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
				RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
				
			
				if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
				{
					BSP_LCD_GLASS_Clear(); 
					BSP_LCD_GLASS_DisplayString((uint8_t *)"RT I X"); 	
				}
	/******************************************************************************************/
	
	
	
	//****3.***** init the time and date
				
				
 		/*****************Students: please complete the following lnes*****************************
		****************************************************************************************/		
				RTC_DateStructure.Year = 0x13;
				RTC_DateStructure.Month = RTC_MONTH_OCTOBER;
				RTC_DateStructure.Date = 0x1E;
				RTC_DateStructure.WeekDay = 0x03;
				
				if(HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN) != HAL_OK)   //BIN format is better 
															//before, must set in BCD format and read in BIN format!!
				{
					BSP_LCD_GLASS_Clear();
					BSP_LCD_GLASS_DisplayString((uint8_t *)"D I X");
				} 
  
  
				RTC_TimeStructure.Hours = 0x0E;  
				RTC_TimeStructure.Minutes = 0x1E;
				RTC_TimeStructure.Seconds = 0x00;
				RTC_TimeStructure.TimeFormat = RTC_HOURFORMAT_24;
				RTC_TimeStructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
				RTC_TimeStructure.StoreOperation = RTC_STOREOPERATION_RESET;
				
				if(HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN) != HAL_OK)   //BIN format is better
																																					//before, must set in BCD format and read in BIN format!!
				{
					BSP_LCD_GLASS_Clear();
					BSP_LCD_GLASS_DisplayString((uint8_t *)"T I X");
				}	
	  
 /********************************************************************************/



				
			__HAL_RTC_TAMPER1_DISABLE(&RTCHandle);
			__HAL_RTC_TAMPER2_DISABLE(&RTCHandle);	
				//Optionally, a tamper event can cause a timestamp to be recorded. ---P802 of RM0090
				//Timestamp on tamper event
				//With TAMPTS set to �1 , any tamper event causes a timestamp to occur. In this case, either
				//the TSF bit or the TSOVF bit are set in RTC_ISR, in the same manner as if a normal
				//timestamp event occurs. The affected tamper flag register (TAMP1F, TAMP2F) is set at the
				//same time that TSF or TSOVF is set. ---P802, about Tamper detection
				//-------that is why need to disable this two tamper interrupts. Before disable these two, when program start, there is always a timestamp interrupt.
				//----also, these two disable function can not be put in the TSConfig().---put there will make  the program freezed when start. the possible reason is
				//-----one the RTC is configured, changing the control register again need to lock and unlock RTC and disable write protection.---See Alarm disable/Enable 
				//---function.
				
			HAL_RTC_WaitForSynchro(&RTCHandle);	
			//To read the calendar through the shadow registers after Calendar initialization,
			//		calendar update or after wake-up from low power modes the software must first clear
			//the RSF flag. The software must then wait until it is set again before reading the
			//calendar, which means that the calendar registers have been correctly copied into the
			//RTC_TR and RTC_DR shadow registers.The HAL_RTC_WaitForSynchro() function
			//implements the above software sequence (RSF clear and RSF check).	
}


void RTC_AlarmAConfig(void)
{
	RTC_AlarmTypeDef RTC_Alarm_Structure;

	//**************students:  you need to set the followint two lines****************
	/********************************************************************************/
	
	RTC_Alarm_Structure.Alarm = RTC_ALARM_A;
  RTC_Alarm_Structure.AlarmMask = RTC_ALARMMASK_ALL;
	
	
	/********************************************************************************/			
  
  if(HAL_RTC_SetAlarm_IT(&RTCHandle,&RTC_Alarm_Structure,RTC_FORMAT_BCD) != HAL_OK)
  {
			BSP_LCD_GLASS_Clear(); 
			BSP_LCD_GLASS_DisplayString((uint8_t *)"A S X");
  }

	__HAL_RTC_ALARM_CLEAR_FLAG(&RTCHandle, RTC_FLAG_ALRAF); //without this line, sometimes(SOMETIMES, when first time to use the alarm interrupt)
																			//the interrupt handler will not work!!! 		

		//need to set/enable the NVIC for RTC_Alarm_IRQn!!!!
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);   
	HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 3, 0);  //not important ,but it is better not use the same prio as the systick
	
}

//You may need to disable and enable the RTC Alarm at some moment in your application
HAL_StatusTypeDef  RTC_AlarmA_IT_Disable(RTC_HandleTypeDef *hrtc) 
{ 
 	// Process Locked  
	__HAL_LOCK(hrtc);
  
  hrtc->State = HAL_RTC_STATE_BUSY;
  
  // Disable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
  
  // __HAL_RTC_ALARMA_DISABLE(hrtc);
    
   // In case of interrupt mode is used, the interrupt source must disabled 
   __HAL_RTC_ALARM_DISABLE_IT(hrtc, RTC_IT_ALRA);


 // Enable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
  
  hrtc->State = HAL_RTC_STATE_READY; 
  
  // Process Unlocked 
  __HAL_UNLOCK(hrtc);  
}


HAL_StatusTypeDef  RTC_AlarmA_IT_Enable(RTC_HandleTypeDef *hrtc) 
{	
	// Process Locked  
	__HAL_LOCK(hrtc);	
  hrtc->State = HAL_RTC_STATE_BUSY;
  
  // Disable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);
  
  // __HAL_RTC_ALARMA_ENABLE(hrtc);
    
   // In case of interrupt mode is used, the interrupt source must disabled 
   __HAL_RTC_ALARM_ENABLE_IT(hrtc, RTC_IT_ALRA);


 // Enable the write protection for RTC registers 
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);
  
  hrtc->State = HAL_RTC_STATE_READY; 
  
  // Process Unlocked 
  __HAL_UNLOCK(hrtc);  

}


/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
	* @reference https://github.com/eylaing/3TA4/blob/master/lab3starter/src/main.c
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  switch (GPIO_Pin) {
			case GPIO_PIN_0: 		               //SELECT button					
						selpressed=1;	
						HAL_RTC_GetTime(&RTCHandle, &RTC_TimeStructure, RTC_FORMAT_BIN);
						HAL_RTC_GetDate(&RTCHandle, &RTC_DateStructure, RTC_FORMAT_BIN);
						if (mode==0){//before enter the modification modes, this part is used to save and read to eeprom
								RTC_AlarmA_IT_Disable(&RTCHandle);
								RTC_SavePressTime();
								EEPROM_SaveTime(RTC_TimeStructure.Hours, RTC_TimeStructure.Minutes, RTC_TimeStructure.Seconds);
						}
						RTC_AlarmA_IT_Enable(&RTCHandle);
						break;	
			case GPIO_PIN_1:     //left button		
							if (mode == 0 && leftpressed == 0) leftpressed ++;
							else if(mode == 0 && (leftpressed == 1||leftpressed ==2)) {
								leftpressed = 0; 
							}
							else if (mode == 1){ //inside the modification modes, left button is used to switch modification item
								modify_mode++;
								if (modify_mode>=7) modify_mode = 1; //modify_mode back to the first "sec" modification
							}
							break;
			case GPIO_PIN_2:    //right button						  to play again.
							
							if(mode==0){
								mode = 1; //before modification modes, right button is used to enter the modification modes
								
							}
							else
								mode = 0; //inside modification modes, right button is used to exit 
							rightpressed=1;
							break;
			case GPIO_PIN_3:    //up button	
					switch(mode){
						case 0:
						break;
						case 1:				
							switch(modify_mode) {
								case 6: //year modify
								{
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_DateStructure.Year++;//adding year
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d", RTC_DateStructure.Year);//change type and store in ShowTemp
									BSP_LCD_GLASS_DisplayString(ShowTemp);//show the year
									HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);					
									break;
								}
								case 5:
								{
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_DateStructure.Month++;//adding month
									if(RTC_DateStructure.Month>12) RTC_DateStructure.Month = 1;//month cannot be greater than 12
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d", RTC_DateStructure.Month);//change type and store in ShowTemp
									BSP_LCD_GLASS_DisplayString(ShowTemp);//show the month
									HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									break;
								}
								case 4:
								{
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_DateStructure.Date++;//adding date
									//the following part is used to modify date by different month type (with 30 days, 31 days, 28 days, and 29 days)
									if(RTC_DateStructure.Month == 1 || (RTC_DateStructure.Month == 3)||(RTC_DateStructure.Month == 5)||(RTC_DateStructure.Month == 7)||(RTC_DateStructure.Month == 8)||(RTC_DateStructure.Month == 10)||(RTC_DateStructure.Month == 12)){
										if(RTC_DateStructure.Date>31) RTC_DateStructure.Date = 0;
									else{
										if(RTC_DateStructure.Date>30) RTC_DateStructure.Date = 0;
										}
									}
									if(RTC_DateStructure.Month == 2){
										if(RTC_DateStructure.Year%4 == 0){
											if(RTC_DateStructure.Date>29) RTC_DateStructure.Date = 0;
										else{
											if(RTC_DateStructure.Date>28) RTC_DateStructure.Date = 0;
											}
										}
									}//finish determining date
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d", RTC_DateStructure.Date);//change type and store in ShowTemp
									BSP_LCD_GLASS_DisplayString(ShowTemp);//show date
									HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									break;
								}
								/*case 4:
								{
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									WDN++;//adding index
									if(WDN > 7) WDN = 0;//index return to 0
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",*Weekday[WDN]);//change type and store in ShowTemp
									BSP_LCD_GLASS_DisplayString(ShowTemp);//show weekday
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}*/
								
								case 3:
								{							
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_TimeStructure.Hours++;//adding hours
									if(RTC_TimeStructure.Hours==24) RTC_TimeStructure.Hours = 0;//24:00 = 00:00
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",RTC_TimeStructure.Hours);//change type and store in ShowTemp
									BSP_LCD_GLASS_DisplayString(ShowTemp);//show hours
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}	
								case 2:
								{
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_TimeStructure.Minutes++;//adding minutes
									if(RTC_TimeStructure.Minutes==60) RTC_TimeStructure.Minutes = 0;//when min = 60, return it to 0
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",RTC_TimeStructure.Minutes);//change type and store in ShowTeMP
									BSP_LCD_GLASS_DisplayString(ShowTemp);//Show min
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}
								case 1:
								{
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_TimeStructure.Seconds++;//adding sec
									if(RTC_TimeStructure.Seconds==60) RTC_TimeStructure.Seconds = 0;//when sec = 60, return it to 0
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",RTC_TimeStructure.Seconds);//change type and store in ShowTemp
									BSP_LCD_GLASS_DisplayString(ShowTemp);//Show sec
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
									}
								}
							}
							break;
								
			case GPIO_PIN_5:    //down button
					switch(mode){
						case 0:
						break;
						case 1:
							BSP_LCD_GLASS_Clear();
							switch(modify_mode) {
								//the following are the same as the up button section but everything is "minus"
								case 6: 
								{
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_DateStructure.Year--;
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d", RTC_DateStructure.Year);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);					
									break;
								}
								case 5:
								{
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_DateStructure.Month--;
									if(RTC_DateStructure.Month == 0) RTC_DateStructure.Month = 12;
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d", RTC_DateStructure.Month);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									break;
								}

								case 4:
								{
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_DateStructure.Date--;
									//determing date by different month type
									if(RTC_DateStructure.Month == 1 || (RTC_DateStructure.Month == 3)||(RTC_DateStructure.Month == 5)||(RTC_DateStructure.Month == 7)||(RTC_DateStructure.Month == 8)||(RTC_DateStructure.Month == 10)||(RTC_DateStructure.Month == 12)){
										if(RTC_DateStructure.Date == 0) RTC_DateStructure.Date = 31;
									else{
										if(RTC_DateStructure.Date == 0) RTC_DateStructure.Date = 30;
										}
									}
									if(RTC_DateStructure.Month == 1){
										if(RTC_DateStructure.Year%4 == 0){
											if(RTC_DateStructure.Date == 0) RTC_DateStructure.Date = 29;
										else{
											if(RTC_DateStructure.Date == 0) RTC_DateStructure.Date = 28;
											}
										}
									}
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d", RTC_DateStructure.Date);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									break;
								}
								/*case 4:
								{
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									WDN--;
									if(WDN < 0) WDN = 7;
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",*Weekday[WDN]);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}*/
								case 3:
								{							
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_TimeStructure.Hours--;
									if(RTC_TimeStructure.Hours==0) RTC_TimeStructure.Hours = 24;
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",RTC_TimeStructure.Hours);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}	
								case 2:
								{
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_TimeStructure.Minutes--;
									if(RTC_TimeStructure.Minutes==0) RTC_TimeStructure.Minutes = 60;
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",RTC_TimeStructure.Minutes);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}
								case 1:
								{
									HAL_RTC_GetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									HAL_RTC_GetDate(&RTCHandle,&RTC_DateStructure,RTC_FORMAT_BIN);
									RTC_TimeStructure.Seconds--;
									if(RTC_TimeStructure.Seconds==0) RTC_TimeStructure.Seconds = 60;
									BSP_LCD_GLASS_Clear();
									sprintf((char*)ShowTemp,"%d",RTC_TimeStructure.Seconds);
									BSP_LCD_GLASS_DisplayString(ShowTemp);
									HAL_RTC_SetTime(&RTCHandle,&RTC_TimeStructure,RTC_FORMAT_BIN);
									break;
								}		
							}
						}
						break;
								
			case GPIO_PIN_14:    //down button						
							BSP_LCD_GLASS_Clear();
							BSP_LCD_GLASS_DisplayString((uint8_t*)"PE14");
							break;			
			default://
						//default
						break;
	  } 
}

/**
  * @brief used to show time
  * @param void
  * @retval None
	* @reference https://github.com/eylaing/3TA4/blob/master/lab3starter/src/main.c
	* @Author eylaing
  */
void RTC_TimeShow(void)
{
	
	BSP_LCD_GLASS_Clear();
  
  /* Get current RTC time & date */
  HAL_RTC_GetTime(&RTCHandle, &RTC_TimeStructure, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&RTCHandle, &RTC_DateStructure, RTC_FORMAT_BIN);
	
	//Format for display: hh:mm:ss
	sprintf((char*)datestring,"%02d%02d%02d", RTC_TimeStructure.Hours, RTC_TimeStructure.Minutes, RTC_TimeStructure.Seconds);
	BSP_LCD_GLASS_DisplayString((uint8_t*)datestring);
	
}

/**
  * @brief used to show date
  * @param void
  * @retval None
	* @reference https://github.com/eylaing/3TA4/blob/master/lab3starter/src/main.c
	* @Author eylaing
  */
void RTC_DateDisplay(void)
{
	/* Get current RTC time & date */
  HAL_RTC_GetTime(&RTCHandle, &RTC_TimeStructure, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&RTCHandle, &RTC_DateStructure, RTC_FORMAT_BIN);
	
	//Format: weekday:day:month:year
	sprintf((char*)datestring," %d-%d-%d-20%d",RTC_DateStructure.WeekDay, RTC_DateStructure.Date, RTC_DateStructure.Month, RTC_DateStructure.Year);
	BSP_LCD_GLASS_ScrollSentence((uint8_t*)datestring,1,150);
}

/**
  * @brief used to save press time
  * @param void
  * @retval None
	* @reference https://github.com/eylaing/3TA4/blob/master/lab3starter/src/main.c
	* @Author eylaing
  */
void RTC_SavePressTime(void)
{
	//use this for saving current time to EEPROM - will need to implement with pushbutton
	/* Get current RTC time & date */
  HAL_RTC_GetTime(&RTCHandle, &RTC_TimeStructure, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&RTCHandle, &RTC_DateStructure, RTC_FORMAT_BIN);
	
	//Need hours, minutes, seconds - save into hh, mm, ss already declared
	hh = RTC_TimeStructure.Hours;
	mm = RTC_TimeStructure.Minutes;
	ss = RTC_TimeStructure.Seconds;

	EEPROM_SaveTime(hh, mm, ss);
	
	
}

/**
  * @brief used to save time into eeprom
  * @param void
  * @retval None
	* @reference https://github.com/eylaing/3TA4/blob/master/lab3starter/src/main.c
	* @Author eylaing
  */
void EEPROM_SaveTime(uint8_t hour, uint8_t min, uint8_t sec)
{
	BSP_LCD_GLASS_Clear();
	BSP_LCD_GLASS_DisplayString((uint8_t*)"SAVING");
	I2C_ByteWrite(&pI2c_Handle, EEPROM_ADDRESS, Memory_Space, hour);
	I2C_ByteWrite(&pI2c_Handle, EEPROM_ADDRESS, Memory_Space+1, min);
	I2C_ByteWrite(&pI2c_Handle, EEPROM_ADDRESS, Memory_Space+2, sec);
	HAL_Delay(500); //longer to display the 'SAVING' long enough for user to read
}

/**
  * @brief used to read time from eeprom
  * @param void
  * @retval None
	* @reference https://github.com/eylaing/3TA4/blob/master/lab3starter/src/main.c
	* @Author eylaing
  */
void EEPROM_ReadTime(uint8_t SPoffset)
{
	uint8_t hourRead, minRead, secRead;
	hourRead=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space-3-SPoffset);
	minRead=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space-2-SPoffset);
	secRead=I2C_ByteRead(&pI2c_Handle,EEPROM_ADDRESS, Memory_Space-1-SPoffset);
	BSP_LCD_GLASS_Clear();
	sprintf((char*)datestring,"%d:%d:%d",hourRead, minRead, secRead);
	BSP_LCD_GLASS_DisplayString((uint8_t*)datestring);
	HAL_Delay(1000); //hold time on screen long enough
}

/**
  * @brief used to save time to I2C
  * @param void
  * @retval None
  */
void I2C_Store_time()
{
	HAL_RTC_GetTime (&RTCHandle,&RTC_TimeStructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate (&RTCHandle,&RTC_DateStructure, RTC_FORMAT_BIN);
	hh = RTC_TimeStructure.Hours;
	mm = RTC_TimeStructure.Minutes;
	ss = RTC_TimeStructure.Seconds;
	I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS,Memory_Space,hh);
	I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS,Memory_Space+1,mm);
	I2C_ByteWrite(&pI2c_Handle,EEPROM_ADDRESS,Memory_Space+2,ss);
	Memory_Space = Memory_Space + 3;
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	switch(mode) {
		case 0:
			switch(SP){
				case 0:
					RTC_TimeShow();//SP will not change, it always show time unless mode == 1
				break;
			}
			switch(leftpressed){
				case 2://show the 2nd stored value, it automatically change the state to case 1.
							BSP_LED_On(LED4);
							BSP_LCD_GLASS_Clear();
							sprintf((char*)ShowTemp2,"%02d%02d%02d",tempData[5],tempData[4],tempData[3]);
							BSP_LCD_GLASS_DisplayString((uint8_t*)ShowTemp2);
							BSP_LCD_GLASS_DisplayChar((uint8_t*)&ShowTemp2[1],POINT_OFF,DOUBLEPOINT_ON,LCD_DIGIT_POSITION_2);
							BSP_LCD_GLASS_DisplayChar((uint8_t*)&ShowTemp2[3],POINT_OFF,DOUBLEPOINT_ON,LCD_DIGIT_POSITION_4);
							leftpressed--;
							BSP_LED_Off(LED4);
							break;
				case 1://show the 1st stored value, it automatically change the state to case 2.
							BSP_LED_On(LED5);
							BSP_LCD_GLASS_Clear();
							sprintf((char*)ShowTemp1,"%02d%02d%02d",tempData[2],tempData[1],tempData[0]);
							BSP_LCD_GLASS_DisplayString((uint8_t*)ShowTemp1);
							BSP_LCD_GLASS_DisplayChar((uint8_t*)&ShowTemp1[1],POINT_OFF,DOUBLEPOINT_ON,LCD_DIGIT_POSITION_2);
							BSP_LCD_GLASS_DisplayChar((uint8_t*)&ShowTemp1[3],POINT_OFF,DOUBLEPOINT_ON,LCD_DIGIT_POSITION_4);
							leftpressed++;
							BSP_LED_Off(LED5);
							break;
				case 0://does nothing
					break;
		}
		break;
		case 1:
			switch(modify_mode){
				case 1://print modification of sec on screen
					BSP_LCD_GLASS_Clear();
					sprintf((char*)ShowTemp,"Sec%d",RTC_TimeStructure.Seconds);//change type and store in ShowTemp
					BSP_LCD_GLASS_DisplayString(ShowTemp);
					break;
				
				case 2://print modification of min on screen
					BSP_LCD_GLASS_Clear();
					sprintf((char*)ShowTemp,"Min%d",RTC_TimeStructure.Minutes);//change type and store in ShowTemp
					BSP_LCD_GLASS_DisplayString(ShowTemp);
					break;
			
				case 3://print modification of hour on screen
					BSP_LCD_GLASS_Clear();
					sprintf((char*)ShowTemp,"Hou%d",RTC_TimeStructure.Hours);//change type and store in ShowTemp
					BSP_LCD_GLASS_DisplayString(ShowTemp);
					break;
			
				case 4:
					BSP_LCD_GLASS_Clear();//print modification of date on screen
					sprintf((char*)ShowTemp,"Date%d", RTC_DateStructure.Date);
					BSP_LCD_GLASS_DisplayString(ShowTemp);
					break;
			
				case 5://print modification of Month on screen
					BSP_LCD_GLASS_Clear();
					sprintf((char*)ShowTemp,"Mnth%d", RTC_DateStructure.Month);
					BSP_LCD_GLASS_DisplayString(ShowTemp);
					break;
			
				case 6://print modification of Year on screen
					BSP_LCD_GLASS_Clear();
					sprintf((char*)ShowTemp,"Y%d", RTC_DateStructure.Year);
					BSP_LCD_GLASS_DisplayString(ShowTemp);
					break;
			}
			break;
	}
}

static void Error_Handler(void)
{
  /* Turn LED4 on */
  BSP_LED_On(LED4);
  while(1)
  {
  }
}




#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
