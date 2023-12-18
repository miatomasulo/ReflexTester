/*
 * LCD_Driver.c
 *
 *  Created on: Nov 30, 2023
 *      Author: miat5
 */

/**
 * @file
 */
#include "LCD_Driver.h"

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor   = 0xFFFF;

static uint32_t totalTime1;
static uint32_t totalTime2;
bool button_Pressed = 0;
bool inLevelThree = false;
int counter = 0;
uint32_t time[3];
uint32_t firstTime3;
uint32_t secondTime3;
uint32_t thirdTime3;
uint32_t aveTime3;
bool validInput = false;

static SPI_HandleTypeDef SpiHandle;
uint32_t SpiTimeout = SPI_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */

//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.

//static void MX_LTDC_Init(void);
//static void MX_SPI5_Init(void);
static void SPI_MspInit(SPI_HandleTypeDef *hspi);
static void SPI_Error(void);

/* Provided Functions and API  - MOTIFY ONLY WITH EXTREME CAUTION!!! */


void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}


void LTCD__Init(void)
{
  	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

void LTCD_Layer_Init(uint8_t LayerIndex)
{
  LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}

}

// Draws a single pixel, should be used only within this file-set and should not be seen by external clients.
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.

}

// Draws Char
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overwrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

// Displays Char
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_DrawChar(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

// Draw Circle Filled
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
  for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

// Draws Vertical Line
void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

//Clears the screen
void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
  if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
}

//Throws code into error handler
void LCD_Error_Handler(void)
{
  for(;;); // Something went wrong
}

// Demo using provided functions
void QuickDemo(void)
{
	uint16_t x;
	uint16_t y;
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	for(y=0; y<LCD_PIXEL_HEIGHT; y++){
		for(x=0; x < LCD_PIXEL_WIDTH; x++){
			if (x & 32)
				frameBuffer[x*y] = LCD_COLOR_WHITE;
			else
				frameBuffer[x*y] = LCD_COLOR_BLACK;
		}
	}

	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_GREEN);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_RED);
	HAL_Delay(1500);
	LCD_Clear(0, LCD_COLOR_WHITE);
	LCD_Draw_Vertical_Line(10,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);
	LCD_Draw_Vertical_Line(230,10,250,LCD_COLOR_MAGENTA);
	HAL_Delay(1500);

	LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_BLACK);
	HAL_Delay(2000);

	LCD_Clear(0,LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(100,140,'H');
	LCD_DisplayChar(115,140,'e');
	LCD_DisplayChar(125,140,'l');
	LCD_DisplayChar(130,140,'l');
	LCD_DisplayChar(140,140,'o');

	LCD_DisplayChar(100,160,'W');
	LCD_DisplayChar(115,160,'o');
	LCD_DisplayChar(125,160,'r');
	LCD_DisplayChar(130,160,'l');
	LCD_DisplayChar(140,160,'d');

}

/*        APPLICATION SPECIFIC FUNCTION DEFINITIONS - PUT YOUR NEWLY CREATED FUNCTIONS HERE       */


// Shapes


/** Draw Square Filled
 *
 */
void LCD_Draw_Square(uint16_t Xpos, uint16_t Ypos, uint16_t len, uint16_t color){
	for (int16_t y=0; y<len; y++){
		for (int16_t x=0; x<len; x++){
			LCD_Draw_Pixel(x+Xpos, y+Ypos, color); //left side

			//use following for no fill square
//			LCD_Draw_Pixel(x+Xpos, Ypos, color); //bottom
//			LCD_Draw_Pixel(len+Xpos, y+Ypos, color); //right sides
//			LCD_Draw_Pixel(x+Xpos, len+Ypos, color); //top
		}
	}
}

/** Draw Triangle Filled
 *
 */
void LCD_Draw_Triangle(uint16_t Xpos, uint16_t Ypos, uint16_t len, uint16_t color){
	for (int16_t y=0; y<len; y++){
		for (int16_t x=0; x<=y; x++){
			LCD_Draw_Pixel(x+Xpos, y+Ypos, color); //BASE
		}
	}
}

/** Draw Horizontal Line
 * @param x gives position on x-axis
 * @param y gives position on y-axis
 * @param len gives length of line
 * @param color specifies color of the displayed lineS
 */
void LCD_Draw_Horizontal_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color){
	for (uint16_t i = 0; i < len; i++)
	{
		LCD_Draw_Pixel(x+i, y, color);
	}
}

/** Checks the status of the button
 *
 */
void buttonStatus(void){

	button_Pressed = true;
}

/** Get function returns a value of true when in Level 3; used to check the conditional in the IRQ handler
 * @retval returns True if code is in Level3() or False if not
 */
bool getLevelThree(void){

	return inLevelThree;
}

/** Get function returns the value of validInput; used to check the conditional in the IRQ handler
 * @retval returns True or False
 */
bool getValidInput(void){

	return validInput;
}

/* Set function sets validInput back to false; used in the IRQ handler
 *
 */
void setValidInput(void){

	validInput = false;
}

/** Increments the count for Level 3 to ensure the user only completes the task 3 times
*Calculates the reaction time of each pass then calculates the average of the 3 times
*/
void incrementCounter(void){
	uint32_t elapsedTime;

	uint32_t endTime = HAL_GetTick();
	if(counter == 0){
		elapsedTime = endTime - firstTime3;
		time[0] = elapsedTime;
	}
	if(counter == 1){
		elapsedTime = endTime - secondTime3;
		time[1] = elapsedTime;
	}
	if(counter == 2){
		elapsedTime = endTime - thirdTime3;
		time[2] = elapsedTime;

		aveTime3 = (time[0] + time[1] + time[2])/3;
	}
	counter++;



}

/** Displays the instructions for Level 1 on the LCD screen, called in main.
 *
 */
void level1Instructions(void){

	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	int i = -15;

	LCD_Clear(0,LCD_COLOR_WHITE);

	char arr1[] = "PRESS THE BUTTON";
	char arr2[] = "WHEN A";
	char arr3[] = "CIRCLE APPEARS";

	for(int j=0; arr1[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,120,arr1[j]);
	}
	i = -15;
	for(int j=0; arr2[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,140,arr2[j]);
	}
	i = -15;
	for(int j=0; arr3[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,160,arr3[j]);
	}

	HAL_Delay(3000);
	LCD_Clear(0,LCD_COLOR_WHITE);
}

/** Displays the instructions for Level 2 on the LCD screen, called in main.
 *
 */
void level2Instructions(void){
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	int i = -15;

	LCD_Clear(0,LCD_COLOR_WHITE);
	HAL_Delay(1000);
	char arr1[] = "PRESS THE BUTTON";
	char arr2[] = "WHEN A SQUARE";
	char arr3[] = "APPEARS IN THE";
	char arr4[] = "TOP RIGHT";
	char arr5[] = "QUADRANT";
	for(int j=0; arr1[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,120,arr1[j]);
	}
	i = -15;
	for(int j=0; arr2[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,140,arr2[j]);
	}
	i = -15;
	for(int j=0; arr3[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,160,arr3[j]);
	}
	i = -15;
	for(int j=0; arr4[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,180,arr4[j]);
	}
	i = -15;
	for(int j=0; arr5[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,200,arr5[j]);
	}

	HAL_Delay(3000);
	LCD_Clear(0,LCD_COLOR_WHITE);
}

/** Displays the instructions for Level 3 on the LCD screen, called in main.
 *
 */
void level3Instructions(void){
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_SetFont(&Font16x24);

	int i = -15;

	LCD_Clear(0,LCD_COLOR_WHITE);
	HAL_Delay(1000);
	char arr1[] = "PRESS THE BUTTON";
	char arr2[] = "WHEN THE BALL";
	char arr3[] = "fULLY CROSSES";
	char arr4[] = "THE LINE";
	char arr5[] = "DO THIS 3 TIMES";
	for(int j=0; arr1[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,120,arr1[j]);
	}
	i = -15;
	for(int j=0; arr2[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,140,arr2[j]);
	}
	i = -15;
	for(int j=0; arr3[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,160,arr3[j]);
	}
	i = -15;
	for(int j=0; arr4[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,180,arr4[j]);
	}
	i = -15;
	for(int j=0; arr5[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,200,arr5[j]);
	}


	HAL_Delay(3000);
	LCD_Clear(0,LCD_COLOR_WHITE);

}

/** Displays the final times for each level at the end of the game; called in Level3().
 * @param unsigned 32-bit values for the times from Level 1, 2, and 3
 */
void timeDisplay(uint32_t totalTime1, uint32_t totalTime2, uint32_t totalTime3){
	char time1[4];
	char time2[4];
	char time3[4];

	itoa(totalTime1, time1, 10);
	itoa(totalTime2, time2, 10);
	itoa(totalTime3, time3, 10);

	char arr1[] = "Level 1 Time";
	char arr2[] = "Level 2 Time";
	char arr3[] = "Level 3 Average";
	char arr32[] = "Time";

	int i = -15;
	for(int j=0; arr1[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,120,arr1[j]);
	}
	i = -15;
	for(int j=0; time1[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,140,time1[j]);
	}
	i = -15;
	for(int j=0; arr2[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,160,arr2[j]);
	}
	i = -15;
	for(int j=0; time2[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,180,time2[j]);
	}
	i = -15;
	for(int j=0; arr3[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,200,arr3[j]);
	}
	i = -15;
	for(int j=0; arr32[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,220,arr32[j]);
	}
	i = -15;
	for(int j=0; time3[j] != '\0'; j++){
		LCD_DisplayChar(i+=15,240,time3[j]);
	}

}
//Game Levels

/** Level 1: Generates a random shape, times the user's reaction to a specific shape (the circle).
 *
 */
void Level1 (void){
	uint32_t time1;
	uint32_t endTime1;

		LCD_Clear(0,LCD_COLOR_WHITE);
		uint32_t RandomNum = 0;

		//Generate random shape;
		while(RandomNum < 66){
			RandomNum = 1+(getRandomNum() % 100); //Sets the range of randomly generated numbers to be between 1 and 100.
		  if (RandomNum<33){

			  LCD_Draw_Square(105,150,40,LCD_COLOR_BLUE2);

			  HAL_Delay(1000);
			  LCD_Clear(0,LCD_COLOR_WHITE);
			  HAL_Delay(500);

		  } else if(RandomNum >= 33 && RandomNum<66){

			  LCD_Draw_Triangle(105,150,40,LCD_COLOR_YELLOW);

			  HAL_Delay(1000);
			  LCD_Clear(0,LCD_COLOR_WHITE);
			  HAL_Delay(500);

		  } else if(RandomNum >= 66){
			  time1 = HAL_GetTick();
			  LCD_Draw_Circle_Fill(125,150,20,LCD_COLOR_MAGENTA);
			  break;
		  }

		}
		button_Pressed = 0;
		while(button_Pressed == 0){

		}
		button_Pressed = 0;
		LCD_Clear(0,LCD_COLOR_WHITE);
		endTime1 = HAL_GetTick();
		totalTime1 = endTime1 - time1;
		HAL_Delay(1000);


}

/** Level 2: Randomly generates a square in 1 of 4 quadrants, times the user's reaction to the square appearing in top right quadrant.
 *
 */
void Level2 (void){

	uint32_t time2;
	uint32_t endTime2;

	uint16_t HALF_LCD_PIXEL_WIDTH = LCD_PIXEL_WIDTH/2;
	uint16_t HALF_LCD_PIXEL_HEIGHT = LCD_PIXEL_HEIGHT/2;

	uint16_t QUARTER_LCD_PIXEL_WIDTH = LCD_PIXEL_WIDTH/4;
	uint16_t QUARTER_LCD_PIXEL_HEIGHT = LCD_PIXEL_HEIGHT/4;

	uint16_t THIRD_QUARTER_LCD_PIXEL_WIDTH = 3*(LCD_PIXEL_WIDTH/4);
	uint16_t THIRD_QUARTER_LCD_PIXEL_HEIGHT = 3*(LCD_PIXEL_HEIGHT/4);

	//Need 4 quadrants: vertical line at x=1/2 width; horizontal line at y=1/2 height
	LCD_Draw_Vertical_Line(HALF_LCD_PIXEL_WIDTH, 0, LCD_PIXEL_HEIGHT, LCD_COLOR_BLACK);
	LCD_Draw_Horizontal_Line(0, HALF_LCD_PIXEL_HEIGHT, LCD_PIXEL_WIDTH, LCD_COLOR_BLACK);


		//Need to generate random shape in 1 quadrant
		uint32_t RandomNum = 0 ;
		//if 1<x<25: first quadrant
		//if 25<x<50: second quadrant
		//if 50<x<75: third quadrant
		//if 75<x<100: fourth quadrant

		while(RandomNum < 75){

			RandomNum = 1+(getRandomNum() % 100);

			if (RandomNum<25){

				LCD_Draw_Square(THIRD_QUARTER_LCD_PIXEL_WIDTH,THIRD_QUARTER_LCD_PIXEL_HEIGHT,40,LCD_COLOR_BLUE2);

				HAL_Delay(1000);
				LCD_Clear(0,LCD_COLOR_WHITE);
				LCD_Draw_Vertical_Line(HALF_LCD_PIXEL_WIDTH, 0, LCD_PIXEL_HEIGHT, LCD_COLOR_BLACK);
				LCD_Draw_Horizontal_Line(0, HALF_LCD_PIXEL_HEIGHT, LCD_PIXEL_WIDTH, LCD_COLOR_BLACK);

				HAL_Delay(500);

			} else if(RandomNum >= 25 && RandomNum<50){

				LCD_Draw_Square(QUARTER_LCD_PIXEL_WIDTH,THIRD_QUARTER_LCD_PIXEL_HEIGHT,40,LCD_COLOR_BLUE2);

				 HAL_Delay(1000);
				 LCD_Clear(0,LCD_COLOR_WHITE);
				 LCD_Draw_Vertical_Line(HALF_LCD_PIXEL_WIDTH, 0, LCD_PIXEL_HEIGHT, LCD_COLOR_BLACK);
			     LCD_Draw_Horizontal_Line(0, HALF_LCD_PIXEL_HEIGHT, LCD_PIXEL_WIDTH, LCD_COLOR_BLACK);

			     HAL_Delay(500);

			} else if(RandomNum >= 50 && RandomNum<75){

				LCD_Draw_Square(QUARTER_LCD_PIXEL_WIDTH,QUARTER_LCD_PIXEL_HEIGHT,40,LCD_COLOR_BLUE2);

				HAL_Delay(1000);
				LCD_Clear(0,LCD_COLOR_WHITE);
				LCD_Draw_Vertical_Line(HALF_LCD_PIXEL_WIDTH, 0, LCD_PIXEL_HEIGHT, LCD_COLOR_BLACK);
				LCD_Draw_Horizontal_Line(0, HALF_LCD_PIXEL_HEIGHT, LCD_PIXEL_WIDTH, LCD_COLOR_BLACK);

				HAL_Delay(500);

			}else if(RandomNum>=75){  //Quad 4
				LCD_Draw_Square(THIRD_QUARTER_LCD_PIXEL_WIDTH,QUARTER_LCD_PIXEL_HEIGHT,40,LCD_COLOR_BLUE2);
				time2 = HAL_GetTick();
				break;
			}
		}
		button_Pressed = 0;
		while(button_Pressed == 0){

		}
		button_Pressed = 0;
		LCD_Clear(0,LCD_COLOR_WHITE);

		endTime2 = HAL_GetTick();
		totalTime2 = endTime2 - time2;
		HAL_Delay(1000);

}


/** Level 3: Circle traverses the screen, times the user's reaction to the ball crossing a centered vertical line
 *
 */
void Level3 (void){

	inLevelThree = true;


	uint16_t length = LCD_PIXEL_WIDTH;

	uint16_t HALF_LCD_PIXEL_WIDTH = LCD_PIXEL_WIDTH/2;

	while(counter < 3){
			for(int i=-((length/2)-30); i<(length/2)-30; i++){//move ball right
				LCD_Draw_Vertical_Line(HALF_LCD_PIXEL_WIDTH, 0, LCD_PIXEL_HEIGHT, LCD_COLOR_BLACK);
				LCD_Draw_Circle_Fill(125+i,150,20,LCD_COLOR_RED);
				HAL_Delay(1);
				LCD_Clear(0, LCD_COLOR_WHITE);
				HAL_Delay(1);

				if(counter == 0 && i == 0){
					firstTime3 = HAL_GetTick();
					validInput = true;
				}else if(counter == 2 && i == 0){
					thirdTime3 = HAL_GetTick();
					validInput = true;
				}

			}
			if(counter != 3){
			for(int j=(length/2)-30; j>-((length/2)-30); j--){ //move ball left
				LCD_Draw_Vertical_Line(HALF_LCD_PIXEL_WIDTH, 0, LCD_PIXEL_HEIGHT, LCD_COLOR_BLACK);
				//start time when ball crosses line
				LCD_Draw_Circle_Fill(125+(j),150,20,LCD_COLOR_RED);
				HAL_Delay(1);
				LCD_Clear(0, LCD_COLOR_WHITE);
				HAL_Delay(1);

				if(counter == 1 && j == 0){
					secondTime3 = HAL_GetTick();
					validInput = true;
				}

			}
		}

		}

	timeDisplay(totalTime1, totalTime2, aveTime3);

}


/* Lower Level Functions for LTCD. 	MOTIFY ONLY WITH EXTREME CAUTION!!  */
static uint8_t Is_LCD_IO_Initialized = 0;




/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
void ili9341_Init(void)
{
  /* Initialize ILI9341 low level bus layer ----------------------------------*/
  LCD_IO_Init();

  /* Configure LCD */
  ili9341_Write_Reg(0xCA);
  ili9341_Send_Data(0xC3);				//param 1
  ili9341_Send_Data(0x08);				//param 2
  ili9341_Send_Data(0x50);				//param 3
  ili9341_Write_Reg(LCD_POWERB); //CF
  ili9341_Send_Data(0x00);				//param 1
  ili9341_Send_Data(0xC1);				//param 2
  ili9341_Send_Data(0x30);				//param 3
  ili9341_Write_Reg(LCD_POWER_SEQ); //ED
  ili9341_Send_Data(0x64);
  ili9341_Send_Data(0x03);
  ili9341_Send_Data(0x12);
  ili9341_Send_Data(0x81);
  ili9341_Write_Reg(LCD_DTCA);
  ili9341_Send_Data(0x85);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x78);
  ili9341_Write_Reg(LCD_POWERA);
  ili9341_Send_Data(0x39);
  ili9341_Send_Data(0x2C);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x34);
  ili9341_Send_Data(0x02);
  ili9341_Write_Reg(LCD_PRC);
  ili9341_Send_Data(0x20);
  ili9341_Write_Reg(LCD_DTCB);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Write_Reg(LCD_FRMCTR1);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x1B);
  ili9341_Write_Reg(LCD_DFC);
  ili9341_Send_Data(0x0A);
  ili9341_Send_Data(0xA2);
  ili9341_Write_Reg(LCD_POWER1);
  ili9341_Send_Data(0x10);
  ili9341_Write_Reg(LCD_POWER2);
  ili9341_Send_Data(0x10);
  ili9341_Write_Reg(LCD_VCOM1);
  ili9341_Send_Data(0x45);
  ili9341_Send_Data(0x15);
  ili9341_Write_Reg(LCD_VCOM2);
  ili9341_Send_Data(0x90);
  ili9341_Write_Reg(LCD_MAC);
  ili9341_Send_Data(0xC8);
  ili9341_Write_Reg(LCD_3GAMMA_EN);
  ili9341_Send_Data(0x00);
  ili9341_Write_Reg(LCD_RGB_INTERFACE);
  ili9341_Send_Data(0xC2);
  ili9341_Write_Reg(LCD_DFC);
  ili9341_Send_Data(0x0A);
  ili9341_Send_Data(0xA7);
  ili9341_Send_Data(0x27);
  ili9341_Send_Data(0x04);

  /* Colomn address set */
  ili9341_Write_Reg(LCD_COLUMN_ADDR);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0xEF);

  /* Page address set */
  ili9341_Write_Reg(LCD_PAGE_ADDR);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x01);
  ili9341_Send_Data(0x3F);
  ili9341_Write_Reg(LCD_INTERFACE);
  ili9341_Send_Data(0x01);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x06);

  ili9341_Write_Reg(LCD_GRAM);
  LCD_Delay(200);

  ili9341_Write_Reg(LCD_GAMMA);
  ili9341_Send_Data(0x01);

  ili9341_Write_Reg(LCD_PGAMMA);
  ili9341_Send_Data(0x0F);
  ili9341_Send_Data(0x29);
  ili9341_Send_Data(0x24);
  ili9341_Send_Data(0x0C);
  ili9341_Send_Data(0x0E);
  ili9341_Send_Data(0x09);
  ili9341_Send_Data(0x4E);
  ili9341_Send_Data(0x78);
  ili9341_Send_Data(0x3C);
  ili9341_Send_Data(0x09);
  ili9341_Send_Data(0x13);
  ili9341_Send_Data(0x05);
  ili9341_Send_Data(0x17);
  ili9341_Send_Data(0x11);
  ili9341_Send_Data(0x00);
  ili9341_Write_Reg(LCD_NGAMMA);
  ili9341_Send_Data(0x00);
  ili9341_Send_Data(0x16);
  ili9341_Send_Data(0x1B);
  ili9341_Send_Data(0x04);
  ili9341_Send_Data(0x11);
  ili9341_Send_Data(0x07);
  ili9341_Send_Data(0x31);
  ili9341_Send_Data(0x33);
  ili9341_Send_Data(0x42);
  ili9341_Send_Data(0x05);
  ili9341_Send_Data(0x0C);
  ili9341_Send_Data(0x0A);
  ili9341_Send_Data(0x28);
  ili9341_Send_Data(0x2F);
  ili9341_Send_Data(0x0F);

  ili9341_Write_Reg(LCD_SLEEP_OUT);
  LCD_Delay(200);
  ili9341_Write_Reg(LCD_DISPLAY_ON);
  /* GRAM start writing */
  ili9341_Write_Reg(LCD_GRAM);
}


/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOn(void)
{
  /* Display On */
  ili9341_Write_Reg(LCD_DISPLAY_ON);
}

/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9341_DisplayOff(void)
{
  /* Display Off */
  ili9341_Write_Reg(LCD_DISPLAY_OFF);
}

/**
  * @brief  Writes  to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void ili9341_Write_Reg(uint8_t LCD_Reg)
{
  LCD_IO_WriteReg(LCD_Reg);
}

/**
  * @brief  Writes data to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void ili9341_Send_Data(uint16_t RegValue)
{
  LCD_IO_WriteData(RegValue);
}

/**
  * @brief  Reads the selected LCD Register.
  * @param  RegValue: Address of the register to read
  * @param  ReadSize: Number of bytes to read
  * @retval LCD Register Value.
  */
uint32_t ili9341_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
  /* Read a max of 4 bytes */
  return (LCD_IO_ReadData(RegValue, ReadSize));
}


/******************************* SPI Routines *********************************/

/**
  * @brief  SPI Bus initialization
  */
static void SPI_Init(void)
{
  if(HAL_SPI_GetState(&SpiHandle) == HAL_SPI_STATE_RESET)
  {
    /* SPI configuration -----------------------------------------------------*/
    SpiHandle.Instance = DISCOVERY_SPI;
    /* SPI baudrate is set to 5.6 MHz (PCLK2/SPI_BaudRatePrescaler = 90/16 = 5.625 MHz)
       to verify these constraints:
       - ILI9341 LCD SPI interface max baudrate is 10MHz for write and 6.66MHz for read
       - l3gd20 SPI interface max baudrate is 10MHz for write/read
       - PCLK2 frequency is set to 90 MHz
    */
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

    /* On STM32F429I-Discovery, LCD ID cannot be read then keep a common configuration */
    /* for LCD and GYRO (SPI_DIRECTION_2LINES) */
    /* Note: To read a register a LCD, SPI_DIRECTION_1LINE should be set */
    SpiHandle.Init.Direction      = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase       = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity    = SPI_POLARITY_LOW;
    SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
    SpiHandle.Init.CRCPolynomial  = 7;
    SpiHandle.Init.DataSize       = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit       = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS            = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode         = SPI_TIMODE_DISABLED;
    SpiHandle.Init.Mode           = SPI_MODE_MASTER;

    SPI_MspInit(&SpiHandle);
    HAL_SPI_Init(&SpiHandle);
  }
}

/**
  * @brief  Reads 4 bytes from device.
  * @param  ReadSize: Number of bytes to read (max 4 bytes)
  * @retval Value read on the SPI
  */
static uint32_t SPI_Read(uint8_t ReadSize)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t readvalue;

  status = HAL_SPI_Receive(&SpiHandle, (uint8_t*) &readvalue, ReadSize, SpiTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    SPI_Error();
  }

  return readvalue;
}

/**
  * @brief  Writes a byte to device.
  * @param  Value: value to be written
  */
static void SPI_Write(uint16_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&SpiHandle, (uint8_t*) &Value, 1, SpiTimeout);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    SPI_Error();
  }
}


/**
  * @brief  SPI error treatment function.
  */
static void SPI_Error(void)
{
  /* De-initialize the SPI communication BUS */
  HAL_SPI_DeInit(&SpiHandle);

  /* Re- Initialize the SPI communication BUS */
  SPI_Init();
}

/**
  * @brief  SPI MSP Init.
  * @param  hspi: SPI handle
  */
static void SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable SPI clock */
  DISCOVERY_SPI_CLK_ENABLE();

  /* Enable DISCOVERY_SPI GPIO clock */
  DISCOVERY_SPI_GPIO_CLK_ENABLE();

  /* configure SPI SCK, MOSI and MISO */
  GPIO_InitStructure.Pin    = (DISCOVERY_SPI_SCK_PIN | DISCOVERY_SPI_MOSI_PIN | DISCOVERY_SPI_MISO_PIN);
  GPIO_InitStructure.Mode   = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull   = GPIO_PULLDOWN;
  GPIO_InitStructure.Speed  = GPIO_SPEED_MEDIUM;
  GPIO_InitStructure.Alternate = DISCOVERY_SPI_AF;
  HAL_GPIO_Init(DISCOVERY_SPI_GPIO_PORT, &GPIO_InitStructure);
}

/********************************* LINK LCD ***********************************/

/**
  * @brief  Configures the LCD_SPI interface.
  */
void LCD_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  if(Is_LCD_IO_Initialized == 0)
  {
    Is_LCD_IO_Initialized = 1;

    /* Configure in Output Push-Pull mode */
    LCD_WRX_GPIO_CLK_ENABLE();
    GPIO_InitStructure.Pin     = LCD_WRX_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LCD_WRX_GPIO_PORT, &GPIO_InitStructure);

    LCD_RDX_GPIO_CLK_ENABLE();
    GPIO_InitStructure.Pin     = LCD_RDX_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LCD_RDX_GPIO_PORT, &GPIO_InitStructure);

    /* Configure the LCD Control pins ----------------------------------------*/
    LCD_NCS_GPIO_CLK_ENABLE();

    /* Configure NCS in Output Push-Pull mode */
    GPIO_InitStructure.Pin     = LCD_NCS_PIN;
    GPIO_InitStructure.Mode    = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull    = GPIO_NOPULL;
    GPIO_InitStructure.Speed   = GPIO_SPEED_FAST;
    HAL_GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);

    /* Set or Reset the control line */
    LCD_CS_LOW();
    LCD_CS_HIGH();

    SPI_Init();
  }
}

/**
  * @brief  Writes register value.
  */
void LCD_IO_WriteData(uint16_t RegValue)
{
  /* Set WRX to send data */
  LCD_WRX_HIGH();

  /* Reset LCD control line(/CS) and Send data */
  LCD_CS_LOW();
  SPI_Write(RegValue);

  /* Deselect: Chip Select high */
  LCD_CS_HIGH();
}

/**
  * @brief  Writes register address.
  */
void LCD_IO_WriteReg(uint8_t Reg)
{
  /* Reset WRX to send command */
  LCD_WRX_LOW();

  /* Reset LCD control line(/CS) and Send command */
  LCD_CS_LOW();
  SPI_Write(Reg);

  /* Deselect: Chip Select high */
  LCD_CS_HIGH();
}

/**
  * @brief  Reads register value.
  * @param  RegValue Address of the register to read
  * @param  ReadSize Number of bytes to read
  * @retval Content of the register value
  */
uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
  uint32_t readvalue = 0;

  /* Select: Chip Select low */
  LCD_CS_LOW();

  /* Reset WRX to send command */
  LCD_WRX_LOW();

  SPI_Write(RegValue);

  readvalue = SPI_Read(ReadSize);

  /* Set WRX to send data */
  LCD_WRX_HIGH();

  /* Deselect: Chip Select high */
  LCD_CS_HIGH();

  return readvalue;
}

/**
  * @brief  Wait for loop in ms.
  * @param  Delay in ms.
  */
void LCD_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}




/*       Generated HAL Stuff      */

//   * @brief LTDC Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_LTDC_Init(void)
// {

//   /* USER CODE BEGIN LTDC_Init 0 */

//   /* USER CODE END LTDC_Init 0 */

//   LTDC_LayerCfgTypeDef pLayerCfg = {0};
//   LTDC_LayerCfgTypeDef pLayerCfg1 = {0};

//   /* USER CODE BEGIN LTDC_Init 1 */

//   /* USER CODE END LTDC_Init 1 */
//   hltdc.Instance = LTDC;
//   hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
//   hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
//   hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
//   hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
//   hltdc.Init.HorizontalSync = 7;
//   hltdc.Init.VerticalSync = 3;
//   hltdc.Init.AccumulatedHBP = 14;
//   hltdc.Init.AccumulatedVBP = 5;
//   hltdc.Init.AccumulatedActiveW = 654;
//   hltdc.Init.AccumulatedActiveH = 485;
//   hltdc.Init.TotalWidth = 660;
//   hltdc.Init.TotalHeigh = 487;
//   hltdc.Init.Backcolor.Blue = 0;
//   hltdc.Init.Backcolor.Green = 0;
//   hltdc.Init.Backcolor.Red = 0;
//   if (HAL_LTDC_Init(&hltdc) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   pLayerCfg.WindowX0 = 0;
//   pLayerCfg.WindowX1 = 0;
//   pLayerCfg.WindowY0 = 0;
//   pLayerCfg.WindowY1 = 0;
//   pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
//   pLayerCfg.Alpha = 0;
//   pLayerCfg.Alpha0 = 0;
//   pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
//   pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
//   pLayerCfg.FBStartAdress = 0;
//   pLayerCfg.ImageWidth = 0;
//   pLayerCfg.ImageHeight = 0;
//   pLayerCfg.Backcolor.Blue = 0;
//   pLayerCfg.Backcolor.Green = 0;
//   pLayerCfg.Backcolor.Red = 0;
//   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   pLayerCfg1.WindowX0 = 0;
//   pLayerCfg1.WindowX1 = 0;
//   pLayerCfg1.WindowY0 = 0;
//   pLayerCfg1.WindowY1 = 0;
//   pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
//   pLayerCfg1.Alpha = 0;
//   pLayerCfg1.Alpha0 = 0;
//   pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
//   pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
//   pLayerCfg1.FBStartAdress = 0;
//   pLayerCfg1.ImageWidth = 0;
//   pLayerCfg1.ImageHeight = 0;
//   pLayerCfg1.Backcolor.Blue = 0;
//   pLayerCfg1.Backcolor.Green = 0;
//   pLayerCfg1.Backcolor.Red = 0;
//   if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN LTDC_Init 2 */

//   /* USER CODE END LTDC_Init 2 */

// }

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
// static void MX_SPI5_Init(void)
// {

//   /* USER CODE BEGIN SPI5_Init 0 */

//   /* USER CODE END SPI5_Init 0 */

//   /* USER CODE BEGIN SPI5_Init 1 */

//   /* USER CODE END SPI5_Init 1 */
//   /* SPI5 parameter configuration*/
//   hspi5.Instance = SPI5;
//   hspi5.Init.Mode = SPI_MODE_MASTER;
//   hspi5.Init.Direction = SPI_DIRECTION_2LINES;
//   hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
//   hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
//   hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
//   hspi5.Init.NSS = SPI_NSS_SOFT;
//   hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
//   hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
//   hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
//   hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
//   hspi5.Init.CRCPolynomial = 10;
//   if (HAL_SPI_Init(&hspi5) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN SPI5_Init 2 */

//   /* USER CODE END SPI5_Init 2 */

// }
