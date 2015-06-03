#include <stdio.h>
#include <stdlib.h>
#define STRICT
#include "gyroClass.h"
using namespace std;

#include "Socket.hpp"
#include "DebugUtility.h"


boost::mutex dir_mutex;
int direction = 4;

void key_thread()
{
    while(1)
    {
        char input = cin.getchar();
        if (input == 'w')
        {
        dir_mutex.lock();
            direction = 0;
        dir_mutex.unlock();
        }
        else if (input == 'a')
        {
        dir_mutex.lock();
            direction = 1;
        dir_mutex.unlock();
        }
        else if (input ==  'd')
        {
        dir_mutex.lock();
            direction = 2;
        dir_mutex.unlock();
        }
        else if (input == 's')
        {
            direction = 3;
        dir_mutex.unlock();
        }
        
        usleep(100000)
        dir_mutex.lock();
        direction = 4;
        dir_mutex.unlock();
        
    }
}

int main(int argc, char** argv)
{
	gyroClass gyro("/dev/ttyUSB1");

	Socket socket;

	if (false == socket.open("172.16.0.2", 8104, Socket::Client, Socket::TCP))
    {
        printf("Failed to open!\n");
        return -1;
    }

	Logger::setLevel(100);
    Logger::out(99, "Will not print!\n");
    Logger::err(101, "Will print!\n");
    Logger::out(Logger::MaxLevel, "This one will definitely print!\n");

	
	while(1)
	{
		float * angles = gyro.getAngles();
		
		float angles2 [5];
		
		angles2[0] = angles[0];
		angles2[1] = angles[1];
		angles2[2] = angles[2];
		
		dir_mutex.lock();
		if (direction == 0)
		{
		    angles[3] = 90;
		    angles[4] = 90;
        }
        else if (direction == 1)
        {
            angles[3] = 90;
		    angles[4] = -90;
        }
        else if (direction == 2)
        {
            angles[3] = -90;
		    angles[4] = 90;
        }		
        else if (direction == 3)
        {
            angles[3] = -90;
		    angles[4] = -90;
        }
        else
        {
            angles[3] = 0;
            angles[4] = 0;
        }
        dir_mutex.unlock();
		cout<<angles[0]<<" "<<angles[1]<<" "<<angles[2]<<endl;

        
		if (false == socket.write((const byte*) angles, 3 * sizeof(float)))
        {
            printf("failed to write the angle!\n");
            return -2;
        }
        
        
        usleep(50000);
		

		delete [] angles;

		
	}
	return 0;
}


/*#include <stdio.h>
#include <stdlib.h>
#define STRICT
#include <tchar.h>
#include "minicom_client.h"

using namespace std;


// extract yaw pitch roll from gyro data
float * parse_gyro_output (string input)
{
	float * gyro_angles = new float [3];

	// look for start of msg
	int start = input.find('=');
	if (start < 0)
		return NULL;

	// look for end of msg
	char temp = 13;
	if (input.find(temp)<0)
		return NULL;

	// cut yaw
	string focus = input.substr(start+1,input.find(temp)+1);
	int end = focus.find(',');
	if (end < 0)
		return NULL;

	gyro_angles[0] = atof(focus.substr(0,end).c_str());
	focus = focus.substr(end+1,focus.find(temp)+1);

	// cut pitch
	end = focus.find(',');
	if (end < 0)
		return NULL;
	gyro_angles[1] = atof(focus.substr(0,end).c_str());
	focus = focus.substr(end+1,focus.find(temp)+1);

	// cut roll
	end = focus.find(temp);
	gyro_angles[2] = atof(focus.substr(0,end).c_str());

	return gyro_angles;
}

int main(int argc, char** argv)
{
#ifdef POSIX 
	termios stored_settings; 
	tcgetattr(0, &stored_settings); 
	termios new_settings = stored_settings; 
	new_settings.c_lflag &= (~ICANON); 
	new_settings.c_lflag &= (~ISIG); // don't automatically handle control-C 
	tcsetattr(0, TCSANOW, &new_settings); 
#endif 
	// set up initialization parameters
	float servo_zeros [3];
	int initcount = 0;

	try 
	{ 
		// opoen ports
		boost::asio::io_service gyro_io; 
		minicom_client gyro_client(gyro_io, boost::lexical_cast<unsigned int>("57600"), "COM8"); 
		boost::thread gyro_thread(boost::bind(&boost::asio::io_service::run, &gyro_io)); 


		string gyro_msg;

		while (1)
		{ 
			gyro_client.out_mutex.lock();
			gyro_msg += gyro_client.out.str();
			gyro_client.out.str("");
			gyro_client.out_mutex.unlock();
			int begin = gyro_msg.find("#");
			int end = gyro_msg.find("#",begin+1);
			if (begin!=end && begin<end)
			{
				float * angles = parse_gyro_output(gyro_msg);

				gyro_msg = gyro_msg.substr(end, gyro_msg.length());
				// check if initialization happened
				if (initcount<10)
				{
					// initialize zero values
					for (unsigned int i = 0; i < 3; i++)
						servo_zeros[i] = angles[i];
					initcount++;
				}
				else
				{
					// go through yaw pitch roll
					for (unsigned int i = 0; i < 3; i++)
					{
						// zero and limit angle values
						angles[i]-=servo_zeros[i];
						if (angles[i]<-90)
							angles[i]=-90;
						else if (angles[i]>90)
							angles[i]=90;

						cout<<angles[i]<<" ";
					}
					cout<<endl;
					// AT THIS POINT ANGLES HAVE BEEN LOADED WITH YAW PITCH AND ROLL
				}
				delete angles;
			}		
		} 
		gyro_client.close(); // close the minicom client connection 
		gyro_thread.join(); // wait for the IO service thread to close 
	} 
	catch (exception& e) 
	{ 
		cerr << "Exception: " << e.what() << "\n"; 
	} 

#ifdef POSIX // restore default buffering of standard input 
	tcsetattr(0, TCSANOW, &stored_settings); 
#endif 
	system("pause");
	return 0;
}

*/

