#include <reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define LED P1

//��Ϊ��������Ǽ��ٲ�����������ٱȵ�1/64 ��
//����N=64ʱ�������������תһȦ
//ʹ��ǰ��̽�J2����ñ��������������Ϊ������9600����У��λ��ֹͣλ1����������Ϊ16������ʽ
uchar code CCW[8]        ={0x08,0x0c,0x04,0x06,0x02,0x03,0x01,0x09};                   //��ʱ����ת�����
uchar code CW[8]        ={0x09,0x01,0x03,0x02,0x06,0x04,0x0c,0x08};                   //��ʱ����ת�����
//uchar code table[]        ={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};        //�������ʾ���ֱ�
sbit P2_0 = P2^0;
sbit P2_1 = P2^1;
sbit P2_2 = P2^2;
sbit P2_3 = P2^3;
sbit FMQ=P3^6;  //������

uchar          flag_rec;
uint        data_rec;
/*********************************************************************
        �жϷ������⹦�ܼĴ�������
*********************************************************************/
void init_interrupt()
{
        TMOD=0x20;                //�ö�ʱ��1���ô��ڲ�����         9600
        TH1=0xfd;                //T1��ʱ��װ��ֵ
        TL1=0xfd;                //T1��ʱ��װ��ֵ
        TR1=1;                    //����T1��ʱ��
        REN=1;          //���ڳ�ʼ��
        SM0=0;                        //�趨���ڹ�����ʽ1
        SM1=1;                        //�趨���ڹ�����ʽ1
        EA=1;           //�������ж�
        ES=1;                        //���������ж�

}

/*************************************************************
                                �����жϺ���
**************************************************************/
void receive() interrupt 4
{
        RI=0;                                        //RI���㣻��Ϊ�յ����ݻ��߻��߷��������ݻ���Ӳ����1
        data_rec=SBUF;                        //��������SBUF����SBUF�е����ݶ��߸�data_rec,��������������ݽ��ա�
        //a=SBUF;
        ES = 0;                                        //�رմ����ж�
        flag_rec=1;                                //�жϱ�־λ��1
}
/*****************************************************************
                                ������ʱ
******************************************************************/
void delaynms(uint aa)
{
  uchar bb;
  while(aa--)
  {   
          for(bb=0;bb<115;bb++)     //1ms��׼��ʱ����
        {    ;   }
  }

}
void delay500us()
{
  int j;
  for(j=0;j<57;j++)
  {
   ;
  }
}
/*******************************************
                                ������
******************************************/
void beep()
{
          uchar t;
          for(t=0;t<100;t++)
          {
                   delay500us();
                 FMQ=!FMQ;   //��������
          }
          FMQ=1;    //�رշ�����
}
/***************************************
                                ����ת���
****************************************/
void motor_ccw_deta(uchar deta,uchar speed)        //�����ʱ��תdeta�� deta=11ʱ30�ȣ�deta=66dʱ180��
{
        uchar i;
        while(deta--)
        {
                for(i=0;i<8;i++)  //����ѭ��һ��
                {
                        P1=CCW[i];
                        delaynms(speed);   //����ת��
                }
        }
}

void motor_cw_deta(uchar deta,uchar speed)        //���˳ʱ��תdeta�� deta=11ʱ30�ȣ�deta=66ʱ180��
{
        uchar i;
        while(deta--)
        {
                 for(i=0;i<8;i++)  //�ڲ�ѭ��һ��
                 {
                           P1=CW[i];
                           delaynms(speed);   //����ת��
                }
        }
}
void Digital_tube_display(uint num)
{
        //4λ�������ʾ����
        
        uchar temp,count;
        temp         = 0;
        count         = 0;
        if(num > 999 || num < 0) return;
        while(num)
        {
                temp         = num % 10;
                P0                 = table[temp];
                switch(count)    //�ж�
                {
                        case 0:P2_3=0;break;  //������1����
                        case 1:P2_2=0;break;  //������2����
                        case 2:P2_1=0;break;  //������3����
                        case 3:P2_0=0;break;  //������4����
                }
                delaynms(5);
                switch(count)    //�ж�
                {
                        case 0:P2_3=1;break;  //������1����
                        case 1:P2_2=1;break;  //������2����
                        case 2:P2_1=1;break;  //������3����
                        case 3:P2_0=1;break;  //������4����
                }
                num         = num / 10;
                count++;
        }
}

/**************************************************************
                                                           �жϴ�����
****************************************************************/
void deal_interrupt_Date(uint dat)
{

        SBUF=dat;                         //��������dat_rec��SBUF��������Ƭ�������ݷ��͵������
        while(!TI);
        TI = 0;

        beep();
        motor_cw_deta(11,1);        //���˳תdeta�ȣ�deta=11ʱ30�ȣ�deta=66ʱ180��
        delaynms(10*dat);                //����Ŀǰ��û���ҵ���δ���255���ϵ����ķ����������ȳ���10
        motor_ccw_deta(11,1);        //�����תdetaȣ�deta=11ʱ30�ȣ�deta=66ʱ180��                        
        
        ES=1;                 //�򿪴����ж�        
        flag_rec=0;
}

/**************************************************************
                                                        ������
****************************************************************/
void main()
{        
        init_interrupt();
        Digital_tube_display(0);
        while(1)
        {
                Digital_tube_display(data_rec);
                P1=0xf0;
                if(flag_rec)        //�յ���������ʱ������
                {
                        deal_interrupt_Date(data_rec);
                }
                
        }
}