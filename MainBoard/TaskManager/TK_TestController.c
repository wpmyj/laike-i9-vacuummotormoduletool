/****************************************************************************************************/
/*			@FileName		:	TK_TestController.c												  	*/
/*			@Author			:	zhengbaiqiu															*/
/*			@Version		:	v1.0																*/
/*			@Date			:	2019.11.12  														*/
/****************************************************************************************************/


/* Includes --------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Project ---------------------------------------------------------------------*/
#include "maindef.h"
#include "bsp.h"
#include "apiLib.h"
#include "TK_TestController.h"


/* Variable --------------------------------------------------------------------*/
/* Function --------------------------------------------------------------------*/
void TK_TestController( void *pvParameters );
void api_SendCMD_To_Display(u8 mode,u8 cmd,u8 status);
u8 api_SendCMD_To_MainBoard(u8 seq,u8 mode,u8 cmd,u16 waittime,u8 type,u8 num);
u8 iiiiiiiii;


/*****************************************************************************
 * @name       :void TK_TestController( void *pvParameters )
 * @date       :2020-09-19 
 * @author     :zhengbaiqiu 
 * @function   :时间控制任务
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void TK_TestController( void *pvParameters )
{
	//api_MachinePowerOn();
	for(;;)
	{
		IWDG_ReloadCounter();
		
// 		PAout(1) = 1;
// 		TIM_SetCompare1(TIM5,13);
// 		PCout(5) = 1;
// 		PAout(0) = 1;
// 		PAout(1) = 1;
// 		vTaskDelay(500);
//		TIM_SetCompare1(TIM5,500);
		
		if(mMaininf.mWork.mWorkKeyPressFlag == 1)
		{
			mMaininf.mWork.mWorkKeyPressFlag = 0;
			mMaininf.mWork.mWorkKeyPressFlag1 = 0;
			
			if(mMaininf.mWork.mWorkChecking == 0)
			{
				mMaininf.mWork.mWorkChecking = 1;
				mMaininf.mWork.mWorkVacuumPWMFlag = 1;
			}
			else
			{
				mMaininf.mWork.mWorkChecking = 0;
				mMaininf.mWork.mWorkVacuumPWMFlag = 0;
			}
		}
		
		if(mMaininf.mWork.mWorkKey1PressFlag == 1)
		{
			mMaininf.mWork.mWorkKey1PressFlag = 0;
			mMaininf.mWork.mWorkKey1PressFlag1 = 0;
			if(mMaininf.mWork.mWorkChecking == 0)
			{
				mMaininf.mWork.mWorkChecking = 1;
				mMaininf.mWork.mWorkVacuumPWMFlag = 2;
			}
			else
			{
				mMaininf.mWork.mWorkChecking = 0;
				mMaininf.mWork.mWorkVacuumPWMFlag = 0;
			}
		}
		
		if(mMaininf.mWork.mWorkChecking == 1)
		{
			if(mMaininf.mWork.mWorkVacuumPWMFlag == 1)
			{
				if(mMaininf.mWork.mWorkVacuumFirst == 0)
				{
					PAout(6) = 1;
					vTaskDelay(500);
					//TIM_SetCompare1(TIM5,500);
					TIM_SetCompare1(TIM5,750);
					//mMaininf.mWork.mWorkVacuumPWMSendFlag = 1;
					mMaininf.mWork.mWorkVacuumFirst = 1;
				}
			}
			else if(mMaininf.mWork.mWorkVacuumPWMFlag == 2)
			{
				if(mMaininf.mWork.mWorkVacuumFirst == 0)
				{
					PAout(6) = 1;
					vTaskDelay(500);
					TIM_SetCompare1(TIM5,750);
					//mMaininf.mWork.mWorkVacuumPWMSendFlag = 2;
					mMaininf.mWork.mWorkVacuumFirst = 1;
				}
			}
		}
		else
		{
			PAout(6) = 0;
			//PAout(0) = 0;
			TIM_SetCompare1(TIM5,0);
			//mMaininf.mWork.mWorkVacuumPWMSendFlag = 0;
			mMaininf.mWork.mWorkVacuumFirst = 0;
		}
	
	}
}

/*****************************************************************************
 * @name       :void api_SendCMD_To_Display(u8 mode,u8 cmd,u8 status)
 * @date       :2020-09-19 
 * @author     :zhengbaiqiu 
 * @function   :send data
 * @parameters :cmd
 * @retvalue   :None
******************************************************************************/
void api_SendCMD_To_Display(u8 mode,u8 cmd,u8 status)
{
	u8 mCont = 0;
	u8 mStatus = 0;
	
	do
	{
		if(++mCont > 3)
		{
			return;
		}
		
		api_UART2_Display_SendCMDData(mode,cmd,status);
		vTaskDelay(1000);
		if((mMaininf.mUart2.mReceiveFlag == 1) && (mMaininf.mUart2.ReceiveBuf[0] == 1) && (mMaininf.mUart2.ReceiveBuf[1] == cmd) &&
		   (mMaininf.mUart2.ReceiveBuf[2] == 1))
		{
			mMaininf.mUart2.mReceiveFlag = 0;
			mStatus = 1;
		}
	}while(mStatus == 0);
	
}

/*****************************************************************************
 * @name       :void api_SendCMD_To_MainBoard(u8 seq,u8 mode,u8 cmd,u16 waittime)
 * @date       :2020-09-19 
 * @author     :zhengbaiqiu 
 * @function   :发送命令到主板
 * @parameters :testtype : 0:读取数据   1:检测发射    2:检测接收   waittime:单次检查时间  
				type:类型0：状态  1：数据     num:重发次数
 * @retvalue   :0:主板无数据返回     1：主板有数据返回
******************************************************************************/
u8 api_SendCMD_To_MainBoard(u8 seq,u8 mode,u8 cmd,u16 waittime,u8 type,u8 num)
{
	u8 mCont = 0;
	
	mMaininf.mUart1.ReceiveBuf[4] = 0;
	
	if(type == 0)
	{
		do{
			if(++mCont > num) return 0;//未接收到数据
			api_UART1_MainBoard_SendCMDData(seq,mode,cmd);
			vTaskDelay(waittime);
		}while((mMaininf.mUart1.mReceiveFlag == 0) || ((mMaininf.mUart1.mReceiveFlag == 1) && (mMaininf.mUart1.ReceiveBuf[4] == 0)));
		
	}
	else
	{
		do{
			if(++mCont > num) return 0;//未接收到数据
			api_UART1_MainBoard_SendCMDData(seq,mode,cmd);
			vTaskDelay(waittime);
		}while(mMaininf.mUart1.mReceiveFlag == 0);
		
	}
	
	mMaininf.mUart1.mReceiveFlag = 0;
	
	return 1;//接收到数据
}

