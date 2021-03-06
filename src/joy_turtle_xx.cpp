#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>



#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>

//串口
#include<stdio.h>      
#include<stdlib.h>     
#include<unistd.h>     
#include<sys/types.h>  
#include<sys/stat.h>   
#include<fcntl.h>      
#include<termios.h>    
#include<errno.h>      
#include<string.h>


int speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
B38400, B19200, B9600, B4800, B2400, B1200, B300, };


int set_opt(int fd3,int nSpeed, int nBits, char nEvent, int nStop)   //配置串口参数
{   //cout<<lat_1<<endl;
    struct termios newtio,oldtio;   
    /*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/   
    if  ( tcgetattr( fd3,&oldtio)  !=  0) 
    {    
        perror("SetupSerial 1");  
        printf("tcgetattr( fd3,&oldtio) -> %d\n",tcgetattr( fd3,&oldtio));   
        return -1;   
    }   
    bzero( &newtio, sizeof( newtio ) );   
    /*步骤一，设置字符大小*/   
    newtio.c_cflag  |=  CLOCAL | CREAD;    
    newtio.c_cflag &= ~CSIZE;    
    /*设置停止位*/   
    switch( nBits )   
    {   
    case 7:   
    newtio.c_cflag |= CS7;   
    break;   
    case 8:   
    newtio.c_cflag |= CS8;   
    break;   
    }   
    /*设置奇偶校验位*/   
    switch( nEvent )   
    {   
    case 'o':  
    case 'O': //奇数   
    newtio.c_cflag |= PARENB;   
    newtio.c_cflag |= PARODD;   
    newtio.c_iflag |= (INPCK | ISTRIP);   
    break;   
    case 'e':  
    case 'E': //偶数   
    newtio.c_iflag |= (INPCK | ISTRIP);   
    newtio.c_cflag |= PARENB;   
    newtio.c_cflag &= ~PARODD;   
    break;  
    case 'n':  
    case 'N':  //无奇偶校验位   
    newtio.c_cflag &= ~PARENB;   
    break;  
    default:  
    break;  
    }   
    /*设置波特率*/   
    switch( nSpeed )   
    {   
    case 2400:   
    cfsetispeed(&newtio, B2400);   
    cfsetospeed(&newtio, B2400);   
    break;   
    case 4800:   
    cfsetispeed(&newtio, B4800);   
    cfsetospeed(&newtio, B4800);   
    break;   
    case 9600:   
    cfsetispeed(&newtio, B9600);   
    cfsetospeed(&newtio, B9600);   
    break;   
    case 115200:   
    cfsetispeed(&newtio, B115200);   
    cfsetospeed(&newtio, B115200);   
    break;   
    case 460800:   
    cfsetispeed(&newtio, B460800);   
    cfsetospeed(&newtio, B460800);   
    break;   
    default:   
    cfsetispeed(&newtio, B9600);   
    cfsetospeed(&newtio, B9600);   
    break;   
    }   
    /*设置停止位*/   
    if( nStop == 1 )   
    newtio.c_cflag &=  ~CSTOPB;   
    else if ( nStop == 2 )   
    newtio.c_cflag |=  CSTOPB;   
    /*设置等待时间和最小接收字符*/   
    newtio.c_cc[VTIME]  = 0;   
    newtio.c_cc[VMIN] = 0;   
    /*处理未接收字符*/   
    tcflush(fd3,TCIFLUSH);   
    /*激活新配置*/   
    if((tcsetattr(fd3,TCSANOW,&newtio))!=0)   
    {   
    perror("com set error");   
    return -1;   
    }   
    printf("set done!\n");   
    return 0;   
} 

/**
*@breif 打开串口
*/
int OpenDev(char *Dev)
{
	int	fd2 = open( Dev, O_RDWR );         //| O_NOCTTY | O_NDELAY
	if (-1 == fd2)
	{ /*设置数据位数*/
		perror("Open Serial Port Fail111111111111111");
		perror("Open Serial Port Fail");
		return -1;
	}
	else
		printf("Open Serial Port Success\n");
	return fd2;

}
int fd1;

void serial_set()    //串口函数的打开与配置
{
	char *dev ="/dev/ttyUSB0";
	fd1 = OpenDev(dev);
	set_opt(fd1, 9600, 8, 'N', 1);
}

unsigned char floatvx[2];
unsigned char floatvw[2];

void long_char(int cmdvel)
{       
	floatvx[0]=cmdvel>>8;
	floatvx[1]=cmdvel;
	//printf("qqqq:%x",floatvx[1]);
}
void long_char1(int cmdvel)
{       
	floatvw[0]=cmdvel>>8;
	floatvw[1]=cmdvel;
	//printf("qqqq:%x",floatvx[1]);
}

unsigned short count_CRC(unsigned char *addr, int num)
{
	unsigned short CRC = 0xFFFF;
	int i;
	while (num--)
	{
		CRC ^= *addr++;
		for (i = 0; i < 8; i++)
		{
		if (CRC & 1)
		{
		CRC >>= 1;
		CRC ^= 0xA001;
		}
		else
		{
		CRC >>= 1;
		}
		}
	}
	return CRC;
}


unsigned char cmd_buff[15];
void set_cmdnum1()
{      
	unsigned short crcs;
	cmd_buff[0]=0xaa;
	cmd_buff[1]=0x55;
	cmd_buff[2]=0x60;
	cmd_buff[3]=floatvx[0];
	cmd_buff[4]=floatvx[1];
	cmd_buff[5]=floatvw[0];
	cmd_buff[6]=floatvw[1];
	crcs=count_CRC(cmd_buff, 7);
	cmd_buff[7]=(crcs&0xff00)>>8;
	cmd_buff[8]=(crcs&0x00ff);
	cmd_buff[9]=0xdd;
	//printf("11111,%x\n",cmd_buff[8]);
	//printf("11111,%x\n",cmd_buff[7]);
}

void write_all(int vx, int vw)   
{
  long_char(vx);
  long_char1(vw);
  set_cmdnum1();

  write(fd1,cmd_buff,10);   //写入下位机数据
}



class TeleopTurtle
{
public:
  TeleopTurtle();

private:
  void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);

  ros::NodeHandle nh_;

  int linear_, angular_;
  double l_scale_, a_scale_;
  ros::Publisher vel_pub_;
  ros::Subscriber joy_sub_;

};



TeleopTurtle::TeleopTurtle():
  linear_(1),
  angular_(2)
{

  nh_.param("axis_linear", linear_, linear_);
  nh_.param("axis_angular", angular_, angular_);
  nh_.param("scale_angular", a_scale_, a_scale_);
  nh_.param("scale_linear", l_scale_, l_scale_);


  vel_pub_ = nh_.advertise<geometry_msgs::Twist>("turtle1/cmd_vel", 1);


  joy_sub_ = nh_.subscribe<sensor_msgs::Joy>("joy", 10, &TeleopTurtle::joyCallback, this);

}


void TeleopTurtle::joyCallback(const sensor_msgs::Joy::ConstPtr& joy)
{
  
  geometry_msgs::Twist twist;



  if(joy->buttons[0]==0 && joy->buttons[1]==0 && joy->buttons[2]==0 && joy->buttons[3]==0 && joy->buttons[6] == 0 || joy->buttons[7] == 0)    //ABXY
  {
      if(joy->buttons[4] == 1  || joy->buttons[5] == 1)   // 刹车
      {
        twist.angular.z = 0;
        twist.linear.x = 0;
        write_all(100*twist.linear.x, 100*twist.angular.z);
      }

     twist.angular.z = a_scale_*joy->axes[angular_];
     twist.linear.x = l_scale_*joy->axes[linear_];
    write_all(100*twist.linear.x, 100*twist.angular.z);   
  }
  if(joy->buttons[6] == 1 || joy->buttons[7] == 1)   //归正
    {
      twist.angular.z = 2.5;
      twist.linear.x = 2.5;
      write_all(100*twist.linear.x, 100*twist.angular.z);
    }
  

//  按键遥控  ABXY
  if(joy->buttons[0] == 1)     // A
  {
    twist.linear.x = -1 ;
    twist.angular.z = 0 ;
   write_all(100*twist.linear.x, 100*twist.angular.z);

  }
 
  if(joy->buttons[3] == 1)     // Y
  {
    twist.linear.x = 1;
    twist.angular.z = 0 ;
    write_all(100*twist.linear.x, 100*twist.angular.z);

  }

  if(joy->buttons[2] == 1)     // x
  {
    twist.linear.x = 0 ;
    twist.angular.z = 1 ;
    write_all(100*twist.linear.x, 100*twist.angular.z);
  }


  if(joy->buttons[1] == 1)     // B
  {
    twist.linear.x = 0 ;
    twist.angular.z = -1 ;
    write_all(100*twist.linear.x, 100*twist.angular.z);
  }
//********************
  vel_pub_.publish(twist);

}


int main(int argc, char** argv)
{
  ros::init(argc, argv, "teleop_turtle");
  serial_set(); 
  TeleopTurtle teleop_turtle;

 

  ros::spin();

}