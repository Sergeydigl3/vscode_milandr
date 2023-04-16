#define USE_MDR1986VE9x // for IntelliSense only (auto defined by Keil uVision)

// custom libs
// #include "display/text.h"
#include "setup.h"

// system libs
// #include "game/doom.h"
#include "stdbool.h"
#include "stdint.h"

volatile uint32_t _delay_counter = 0;

extern "C" void SysTick_Handler()
{
	if (_delay_counter)
		_delay_counter--;
}

void delay_ms(uint32_t delay)
{
	_delay_counter = delay;
	while (_delay_counter)
		;
}

void display(uint8_t n)
{
	((n & 0x01) ? PORT_SetBits : PORT_ResetBits)(MDR_PORTD, PORT_Pin_10);
	((n & 0x02) ? PORT_SetBits : PORT_ResetBits)(MDR_PORTD, PORT_Pin_11);
	((n & 0x04) ? PORT_SetBits : PORT_ResetBits)(MDR_PORTD, PORT_Pin_12);
	((n & 0x08) ? PORT_SetBits : PORT_ResetBits)(MDR_PORTD, PORT_Pin_13);
	((n & 0x10) ? PORT_SetBits : PORT_ResetBits)(MDR_PORTD, PORT_Pin_14);
}

void delay(uint32_t ticks)
{
	while (ticks--)
		;
}

void buffer_graphics(void)
{
	int i;
	// cross lines
	Buffer_Line(0, 0, 127, 63, true);
	Buffer_Line(127, 0, 0, 63, true);
	// corner circles
	Buffer_Circle(0, 0, 16, true);
	Buffer_Circle(0, 63, 16, true);
	Buffer_Circle(127, 0, 16, true);
	Buffer_Circle(127, 63, 16, true);
	// center circles
	Buffer_Circle(53, 32, 22, true);
	Buffer_Circle(74, 31, 22, true);
	// side lines
	Buffer_Line(22, 11, 22, 52, true);
	Buffer_Line(105, 11, 105, 52, true);
	DrawBuffer();
	// delay(5000000);
	delay_ms(2000);

	// growing circles
	for (i = 0; i < 32; i++)
	{
		Buffer_Circle(53, 32, i, true);
		Buffer_Circle(74, 31, i, true);
		DrawBuffer();
		delay_ms(50);
	}
}

void draw_text(void)
{
	int i = 0, j, len = 16;
	Buffer_Line(8, 8, 127, 8, true);
	Buffer_Line(8, 19, 127, 19, true);
	Buffer_Line(8, 8, 8, 19, true);
	Buffer_Text(10, 10, "\xCF\xEE\xF2\xE0\xF2"); // Потат
	DrawBuffer();
	delay_ms(1500);

	while (true)
	{
		Buffer_Line(8, 8, 127, 8, true);
		Buffer_Line(8, 19, 127, 19, true);
		Buffer_Line(8, 8, 8, 19, true);
		for (j = 0; j < len; j++)
			Buffer_Char(10 + j * CurrentFont->Width, 10,
						(i + j) % CurrentFont->Count);
		i++;
		DrawBuffer();
		delay_ms(200);
	}
}

// int main(void)
// {
// 	setup();

// 	while (true)
// 	{
// 		buffer_graphics();
// 		delay_ms(1000);
// 		draw_text();
// 		delay_ms(1000);
// 	}
// }

// --- new code --- //

// int main_old_2(void)
// {
// 	int i, x, y, clip_height;
// 	char a[2] = {'0', 0};

// 	setup();

// 	while (true)
// 	{
// 		i = 0;

// 		while (i < 8)
// 		{
// 			fadeScreen(i++ % 8, true);
// 			DrawBuffer();
// 			delay_ms(200);
// 		}

// 		while (i < 16)
// 		{
// 			y = i++;
// 			x = y;
// 			clip_height = max(0, min(y + BMP_GUN_HEIGHT, RENDER_HEIGHT) - y);
// 			drawBitmap(x, y, bmp_gun_mask, BMP_GUN_WIDTH, clip_height, 0);
// 			drawBitmap(x, y, bmp_gun_bits, BMP_GUN_WIDTH, clip_height, 1);
// 			DrawBuffer();
// 			delay_ms(150);
// 		}

// 		drawText(0, 0, "Hello, world!");
// 		DrawBuffer(false);
// 		delay_ms(1500);

// 		drawText(20, 20, "lol kek cheburek =)");
// 		DrawBuffer();
// 		delay_ms(1500);

// 		for (i = 0; i < 10; i++)
// 		{
// 			Buffer_Text(0, 30, "Hello, world!", i);
// 			a[0] = i + '0';
// 			Buffer_Text(0, 40, a);
// 			DrawBuffer();
// 			delay_ms(200);
// 		}
// 	}
// }

/// --- doom code --- ///

// int main(void)
// {
// 	setup();
// 	// while (true)
// 		// loop();
// }

// int main(void) {
//   int i = 0;
//   setup();
//   while (true) {
//     if (input_fire()) i++;
//     Buffer_Char(0, 0, i % 256);
//     Buffer_Text(0, 10, "Hello, world!");
//     DrawBuffer();
//     delay_ms(100);
//   }
// }


USB_Clock_TypeDef USB_Clock_InitStruct;
USB_DeviceBUSParam_TypeDef USB_DeviceBUSParam;

static uint8_t Buffer[BUFFER_LENGTH];

#ifdef USB_CDC_LINE_CODING_SUPPORTED
static USB_CDC_LineCoding_TypeDef LineCoding;
#endif /* USB_CDC_LINE_CODING_SUPPORTED */

#ifdef USB_VCOM_SYNC
volatile uint32_t PendingDataLength = 0;
#endif /* USB_VCOM_SYNC */

/* USB protocol debugging */
#ifdef USB_DEBUG_PROTO

#define USB_DEBUG_NUM_PACKETS   100

typedef struct {
	USB_SetupPacket_TypeDef packet;
	uint32_t address;
} TDebugInfo;

static TDebugInfo SetupPackets[USB_DEBUG_NUM_PACKETS];
static uint32_t SPIndex;
static uint32_t ReceivedByteCount, SentByteCount, SkippedByteCount;

#endif /* USB_DEBUG_PROTO */

/* Private function prototypes -----------------------------------------------*/
static void Setup_CPU_Clock ( void );
static void Setup_USB ( void );
static void VCom_Configuration ( void );

/* Private functions ---------------------------------------------------------*/

/* Main function - initialization and empty infinite loop */
#ifdef __CC_ARM
int main(void)
#else
void main ( void )
#endif
{
	VCom_Configuration();

	/* CDC layer initialization */
	USB_CDC_Init(Buffer, 1, SET);

	Setup_CPU_Clock();
	Setup_USB();

	/* Main loop */
	while (1) {
	}

}

/* Frequencies setup */
void Setup_CPU_Clock ( void )
{
	/* Enable HSE */
	RST_CLK_HSEconfig(RST_CLK_HSE_ON );
	if (RST_CLK_HSEstatus() != SUCCESS) {
		/* Trap */
		while (1) {
		}
	}

	/* CPU_C1_SEL = HSE */
	RST_CLK_CPU_PLLconfig(RST_CLK_CPU_PLLsrcHSEdiv1, RST_CLK_CPU_PLLmul10 );
	RST_CLK_CPU_PLLcmd(ENABLE);
	if (RST_CLK_CPU_PLLstatus() != SUCCESS) {
		/* Trap */
		while (1) {
		}
	}

	/* CPU_C3_SEL = CPU_C2_SEL */
	RST_CLK_CPUclkPrescaler(RST_CLK_CPUclkDIV1 );
	/* CPU_C2_SEL = PLL */
	RST_CLK_CPU_PLLuse(ENABLE);
	/* HCLK_SEL = CPU_C3_SEL */
	RST_CLK_CPUclkSelection(RST_CLK_CPUclkCPU_C3 );
}

/* USB Device layer setup and powering on */
void Setup_USB ( void )
{
	/* Enables the CPU_CLK clock on USB */
	RST_CLK_PCLKcmd(RST_CLK_PCLK_USB, ENABLE);

	/* Device layer initialization */
	USB_Clock_InitStruct.USB_USBC1_Source = USB_C1HSEdiv2;
	USB_Clock_InitStruct.USB_PLLUSBMUL = USB_PLLUSBMUL12;

	USB_DeviceBUSParam.MODE = USB_SC_SCFSP_Full;
	USB_DeviceBUSParam.SPEED = USB_SC_SCFSR_12Mb;
	USB_DeviceBUSParam.PULL = USB_HSCR_DP_PULLUP_Set;

	USB_DeviceInit(&USB_Clock_InitStruct, &USB_DeviceBUSParam);

	/* Enable all USB interrupts */
	USB_SetSIM(USB_SIS_Msk);

	USB_DevicePowerOn();

	/* Enable interrupt on USB */
#ifdef USB_INT_HANDLE_REQUIRED
	NVIC_EnableIRQ(USB_IRQn);
#endif /* USB_INT_HANDLE_REQUIRED */

	USB_DEVICE_HANDLE_RESET;
}

/* Example-relating data initialization */
static void VCom_Configuration ( void )
{
#ifdef USB_CDC_LINE_CODING_SUPPORTED
	LineCoding.dwDTERate = 115200;
	LineCoding.bCharFormat = 0;
	LineCoding.bParityType = 0;
	LineCoding.bDataBits = 8;
#endif /* USB_CDC_LINE_CODING_SUPPORTED */
}

/* USB_CDC_HANDLE_DATA_RECEIVE implementation - data echoing */
USB_Result USB_CDC_RecieveData ( uint8_t* Buffer, uint32_t Length )
{
	USB_Result result;

#ifdef USB_DEBUG_PROTO
	ReceivedByteCount += Length;
#endif /* USB_DEBUG_PROTO */

	/* Send back received data portion */
	result = USB_CDC_SendData(Buffer, Length);

#ifdef USB_DEBUG_PROTO
	if (result == USB_SUCCESS) {
		SentByteCount += Length;
	}
#ifndef USB_VCOM_SYNC
	else
	{
		SkippedByteCount += Length;
	}
#endif /* !USB_VCOM_SYNC */
#endif /* USB_DEBUG_PROTO */

#ifdef USB_VCOM_SYNC
	if (result != USB_SUCCESS) {
		/* If data cannot be sent now, it will await nearest possibility
		 * (see USB_CDC_DataSent) */
		PendingDataLength = Length;
	}
	return result;
#else
	return USB_SUCCESS;
#endif /* USB_VCOM_SYNC */
}

#ifdef USB_VCOM_SYNC

/* USB_CDC_HANDLE_DATA_SENT implementation - sending of pending data */
USB_Result USB_CDC_DataSent ( void )
{
	USB_Result result = USB_SUCCESS;

	if (PendingDataLength) {
		result = USB_CDC_SendData(Buffer, PendingDataLength);
#ifdef USB_DEBUG_PROTO
		if (result == USB_SUCCESS) {
			SentByteCount += PendingDataLength;
		}
		else {
			SkippedByteCount += PendingDataLength;
		}
#endif /* USB_DEBUG_PROTO */
		PendingDataLength = 0;
		USB_CDC_ReceiveStart();
	}
	return USB_SUCCESS;
}

#endif /* USB_VCOM_SYNC */

#ifdef USB_CDC_LINE_CODING_SUPPORTED

/* USB_CDC_HANDLE_GET_LINE_CODING implementation example */
USB_Result USB_CDC_GetLineCoding ( uint16_t wINDEX,
		USB_CDC_LineCoding_TypeDef* DATA )
{
	assert_param(DATA);
	if (wINDEX != 0) {
		/* Invalid interface */
		return USB_ERR_INV_REQ;
	}

	/* Just store received settings */
	*DATA = LineCoding;
	return USB_SUCCESS;
}

/* USB_CDC_HANDLE_SET_LINE_CODING implementation example */
USB_Result USB_CDC_SetLineCoding ( uint16_t wINDEX,
		const USB_CDC_LineCoding_TypeDef* DATA )
{
	assert_param(DATA);
	if (wINDEX != 0) {
		/* Invalid interface */
		return USB_ERR_INV_REQ;
	}

	/* Just send back settings stored earlier */
	LineCoding = *DATA;
	return USB_SUCCESS;
}

#endif /* USB_CDC_LINE_CODING_SUPPORTED */

#ifdef USB_DEBUG_PROTO

/* Overwritten USB_DEVICE_HANDLE_SETUP default handler - to dump received setup packets */
USB_Result USB_DeviceSetupPacket_Debug ( USB_EP_TypeDef EPx,
		const USB_SetupPacket_TypeDef* USB_SetupPacket )
{

#ifdef USB_DEBUG_PROTO
	SetupPackets[SPIndex].packet = *USB_SetupPacket;
	SetupPackets[SPIndex].address = USB_GetSA();
	SPIndex = (SPIndex < USB_DEBUG_NUM_PACKETS ? SPIndex + 1 : 0);
#endif /* USB_DEBUG_PROTO */

	return USB_DeviceSetupPacket(EPx, USB_SetupPacket);
}

#endif /* USB_DEBUG_PROTO */

/**
 * @brief  Reports the source file ID, the source line number
 *         and expression text (if USE_ASSERT_INFO == 2) where
 *         the assert_param error has occurred.
 * @param  file_id: pointer to the source file name
 * @param  line: assert_param error line source number
 * @param  expr:
 * @retval None
 */
#if (USE_ASSERT_INFO == 1)
void assert_failed(uint32_t file_id, uint32_t line)
{
	/* User can add his own implementation to report the source file ID and line number.
	 Ex: printf("Wrong parameters value: file Id %d on line %d\r\n", file_id, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#elif (USE_ASSERT_INFO == 2)
void assert_failed(uint32_t file_id, uint32_t line, const uint8_t* expr);
{
	/* User can add his own implementation to report the source file ID, line number and
	 expression text.
	 Ex: printf("Wrong parameters value (%s): file Id %d on line %d\r\n", expr, file_id, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif /* USE_ASSERT_INFO */

/** @} *//* End of group USB_Virtual_COM_Port_Echo_1T */

/** @} *//* End of group __MDR1986VE1T_EVAL */

/** @} *//* End of group __MDR32F9Qx_StdPeriph_Examples */

/******************* (C) COPYRIGHT 2013 Milandr *******************
 *
 * END OF FILE main.c */

