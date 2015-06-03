// this class when created pools the IMU and parses the angles

#pragma once
#include "minicom_client.h"
using namespace std;

class gyroClass 
{ 
public: 
	gyroClass(string portname)
	{
		try
		{
			// open ports
			gyro_client = new minicom_client (gyro_io, boost::lexical_cast<unsigned int>("57600"), portname); 
			gyro_thread = boost::thread(boost::bind(&boost::asio::io_service::run, &gyro_io)); 

			// initialize
			int initcount = 0;
			

			string gyro_msg;

			// read IMU a few times since first few values are garbage
			while (1)
			{ 
				// clear buffer
				gyro_client->out_mutex.lock();
				gyro_msg += gyro_client->out.str();
				gyro_client->out.str("");
				gyro_client->out_mutex.unlock();

				// parse data
				int begin = gyro_msg.find("#");
				int end = gyro_msg.find("#",begin+1);
				if (begin!=end && begin<end)
				{
					float * temp_angles = parse_gyro_output(gyro_msg);

					gyro_msg = gyro_msg.substr(end, gyro_msg.length());
					// check if initialization happened
					if (initcount<20)
					{
						// initialize zero values
						for (unsigned int i = 0; i < 3; i++)
							servo_zeros[i] = temp_angles[i];
						initcount++;
					}
					else
						break;
				}
			}
			//angle_update_mutex.initialize();
			read_thread = boost::thread(boost::bind(&gyroClass::read, this));
		} 
		catch (exception& e) 
		{ 
			cerr << "Exception: " << e.what() << "\n"; 
		} 
	}

	// return angles
	float * getAngles()
	{
		float * output = new float [3];
		angle_update_mutex.lock();
		for (unsigned int i = 0; i < 3; i++)
			output[i] = angles[i];
		angle_update_mutex.unlock();
		return output;
	}

	void close()
	{
		gyro_client->close(); // close the minicom client connection 
		gyro_thread.join(); // wait for the IO service thread to close 
		read_thread.join();
		delete gyro_client;
	}

private:
	// set up initialization parameters
	float servo_zeros [3];
	float angles [3];
	minicom_client * gyro_client;
	boost::asio::io_service gyro_io;
	boost::mutex angle_update_mutex;
	boost::thread gyro_thread;
	boost::thread read_thread;

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
		string focus = input.substr(start+1,input.find(temp,start)+1);
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

	// thread to pool IMU
	void read()
	{
		try
		{
			string gyro_msg;
			while(1)
			{
				// clear buffer
				gyro_client->out_mutex.lock();
				gyro_msg += gyro_client->out.str();
				gyro_client->out.str("");
				gyro_client->out_mutex.unlock();

				// parse data
				int begin = gyro_msg.find("#");
				int end = gyro_msg.rfind("#");
				if (begin!=end && begin<end && end != gyro_msg.npos)
				{
					float * temp_angles = parse_gyro_output(gyro_msg);

					if (temp_angles==NULL)
					{
						cout<<"NULL ERROR"<<endl;
					}

					gyro_msg = gyro_msg.substr(end, gyro_msg.length());

					// go through yaw pitch roll
					for (unsigned int i = 0; i < 3; i++)
					{
						// zero and limit angle values
						temp_angles[i]-=servo_zeros[i];
                        /*
						if (temp_angles[i]<-90)
							temp_angles[i]=-90;
						else if (temp_angles[i]>90)
                        temp_angles[i]=90;*/
						if (temp_angles[i]<0)
							temp_angles[i]+=360;
						else if (temp_angles[i]>=360)
							temp_angles[i]-=360;
					}
					// AT THIS POINT temp_angles HAVE BEEN LOADED WITH YAW PITCH AND ROLL

					// update angle buffer
					angle_update_mutex.lock();
					for (unsigned int i = 0; i < 3; i++)
						angles[i] = temp_angles[i];
					angle_update_mutex.unlock();

					delete temp_angles;
				}	

			}
		} 
		catch (exception& e) 
		{ 
			cerr << "Exception: " << e.what() << "\n"; 
		} 
	}
};
