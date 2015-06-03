#pragma once
#include "minicom_client.h"
using namespace std;

class servoClass
{
public:
	servoClass(string portname)
	{
		try
		{
			// open ports
			servo_client = new minicom_client (servo_io, boost::lexical_cast<unsigned int>("38400"), portname); 
			servo_thread = boost::thread(boost::bind(&boost::asio::io_service::run, &servo_io)); 
		} 
		catch (exception& e) 
		{ 
			cerr << "Exception: " << e.what() << "\n"; 
		} 

		// data packet format
		data[0] = 128;
		data[1] = 1;
		data[2] = 4;
		data[3] = 1;
		data[6] = 128;
		data[7] = 1;
		data[8] = 4;
		data[9] = 2;
		data[12] = 128;
		data[13] = 1;
		data[14] = 4;
		data[15] = 3;
		//write_mutex.initialize();
		//write_mutex2.initialize();
		isWriting = 0;
	}

	// send angles to motor controller
	void writeAngles(const float * input)
	{
		write_mutex.lock();
		if (isWriting == 0) // do not write if already writing to server
		{
			// load buffer
			write_mutex2.lock();
			for (unsigned int i = 0; i < 3; i++)
			{
				angles[i] = input[i];
			}
			write_mutex2.unlock();
			isWriting = 1;
			
			// initiate non blocking write
			boost::thread (boost::bind(&servoClass::write,this));
		}
		write_mutex.unlock();
		
	}

	void close()
	{
		servo_client->close(); // close the minicom client connection 
		servo_thread.join(); // wait for the IO service thread to close 
		delete servo_client;
	}

private:
	char data [18];
	float angles [3];
	bool isWriting;
	minicom_client * servo_client;
	boost::asio::io_service servo_io;
	boost::thread servo_thread;
	boost::mutex write_mutex;
	boost::mutex write_mutex2;

	// write thread
	void write()
	{
		// go through yaw pitch roll
		write_mutex2.lock();
		for (unsigned int i = 0; i < 3; i++)
		{
			// parse angles to motor signals
			char * motor_speed = generate_motor_speed(angles[i]/0.9);

			// setup data packet
			data[i*6+4] = motor_speed[1];
			data[i*6+5] = motor_speed[0];

			delete motor_speed;
		}		
		write_mutex2.unlock();
		servo_client->write(data, 18);

		write_mutex.lock();
		isWriting = 0; // finished writing
		write_mutex.unlock();
	}

	// parse number from -100 to 100 into a signal for motor controller
	char * generate_motor_speed(const float input)
	{
		char * data = new char [2];
		float motor_speed = input;

		// limit input
		if (input > 100)
			motor_speed = 100;
		else if (input < -100)
			motor_speed = -100;

		motor_speed*=-1;
		// change range to 500 to 5500
		unsigned int temp = (unsigned int) (((motor_speed/2)+50)/100 * 4998 + 501);
		assert(temp>500);
		assert(temp<5500);
		//cout<<temp<<" ";

		// parse data
		data[1] = 0;
		if (temp >= 4096)
		{
			temp -= 4096;
			data[1]+= 32;
		}
		if (temp >= 2048)
		{
			temp -= 2048;
			data[1]+= 16;
		}
		if (temp >= 1024)
		{
			temp -= 1024;
			data[1]+= 8;
		}
		if (temp >= 512)
		{
			temp -= 512;
			data[1]+= 4;
		}
		if (temp >= 256)
		{
			temp -= 256;
			data[1]+= 2;
		}
		if (temp >= 128)
		{
			temp -= 128;
			data[1]+= 1;
		}

		data[0] = (char)temp;
		return data;
	}
};
