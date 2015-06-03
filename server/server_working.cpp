#include <stdio.h>
#include <stdlib.h>
#define STRICT
#include "gyroClass.h"
using namespace std;

#include "Socket.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc_c.h>

#include "DebugUtility.h"
#include "VideoStream.h"

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>

Socket socket2;
gyroClass gyro("/dev/ttyUSB0");

int upkey = 0;
int downkey = 0;
int leftkey = 0;
int rightkey = 0;
int resetFlag = 0;

std::string addr1, addr2;

bool isRunning = true;
float compass_heading;
boost::mutex compass_heading_mutex;
cv::Mat buffer1, buffer2;
boost::mutex mutex1, mutex2;

   
void overlayCompass(IplImage* image, float compass_heading)
{
    assert(compass_heading < 360);
    assert(compass_heading >= 0);
    compass_heading = compass_heading-90;

    int thickness = -1;
    int lineType = 8;
    CvPoint center = cvPoint (300, 215);
    cvCircle( image,
              center,
              15,
              cvScalar( 0, 0, 0 ),
              thickness,
              lineType );

    cvCircle( image,
              center,
              14,
              cvScalar( 255, 255, 255 ),
              thickness,
              lineType );

    CvPoint tip = cvPoint (300 + 12*cos(compass_heading/360*2*3.1418), 215 + 12*sin(compass_heading/360*2*3.1418));
    cvLine(image, center, tip, cvScalar( 0, 0, 255 ), 3, 8, 0);
}

IplImage * generate3D(IplImage * imgLeft, IplImage* imgRight, bool option)
{
    IplImage *iplReturn;
    if (option)
    {
        IplImage *l_R, * l_G, *l_B;
        IplImage *r_R, * r_G, *r_B;

        iplReturn = cvCreateImage( cvGetSize (imgLeft), 8, 3);

        l_R = cvCreateImage( cvGetSize (imgLeft), 8, 1);
        l_G = cvCreateImage( cvGetSize (imgLeft), 8, 1);
        l_B = cvCreateImage( cvGetSize (imgLeft), 8, 1);
        r_R = cvCreateImage( cvGetSize (imgLeft), 8, 1);
        r_G = cvCreateImage( cvGetSize (imgLeft), 8, 1);
        r_B = cvCreateImage( cvGetSize (imgLeft), 8, 1);

        cvSplit(imgLeft, l_B, l_G, l_R, NULL);
        cvSplit(imgRight, r_B, r_G, r_R, NULL);

        cvMerge(r_R, r_G, l_B, NULL, iplReturn);

        cvReleaseImage(&l_R);
        cvReleaseImage(&l_G);
        cvReleaseImage(&l_B);
        cvReleaseImage(&r_R);
        cvReleaseImage(&r_G);
        cvReleaseImage(&r_B);
    }
    else
    {
        iplReturn = cvCreateImage( cvSize (640,480), 8, 3);
        IplImage * smallLeft = cvCreateImage( cvSize (640,240), 8, 3);
        IplImage * smallRight = cvCreateImage( cvSize (640,240), 8, 3);
        cvResize(imgLeft, smallLeft);
        cvResize(imgRight, smallRight);


        for(unsigned int i=0;i<240;i++)
        {
            for (unsigned int j = 0; j<iplReturn->widthStep; j++)
                iplReturn->imageData[2*i*iplReturn->widthStep+j]= smallLeft->imageData[i*iplReturn->widthStep+j];
            for (unsigned int j = 0; j<iplReturn->widthStep; j++)
                iplReturn->imageData[(2*i+1)*iplReturn->widthStep+j]= smallRight->imageData[i*iplReturn->widthStep+j];
        }

        cvReleaseImage(&smallLeft);
        cvReleaseImage(&smallRight);
    }
    return iplReturn;
}


VideoStream stream1;
VideoStream stream2;
void video1()
{
    

    
    cv::Mat video_buffer1;

	printf("1 is grabbing frame\n");
        if (false == stream1.grabFrame(video_buffer1))
        {
            printf("Cannot Grab 1\n");
     
        }
        else
            printf("other error 1\n");
        printf("1 is attempting lock\n");
        mutex1.lock();
        buffer1 = video_buffer1.clone();
        printf("1                    %d, %d\n", buffer1.data==NULL, video_buffer1.data==NULL);
        mutex1.unlock();
   
}

void video2()
{
    

    cv::Mat video_buffer2;
  	printf("2 is grabbing frame\n");
        if (false == stream2.grabFrame(video_buffer2))
        {
            printf("Cannot Grab 2\n");
      
        }
        else
            printf("other error 2\n");

        printf("2 is attempting lock\n");
        mutex2.lock();
        buffer2 = video_buffer2.clone();
        printf("2                    %d, %d\n", buffer2.data==NULL, video_buffer2.data==NULL);
        mutex2.unlock();
    
    
}

void thread_video ()
{
	

    if (false == stream1.open(addr1.c_str()))
    {
        Logger::out(Logger::MaxLevel, "stream1 failed to open\n");
        exit(0);
    }
	if (false == stream2.open(addr2.c_str()))
    {
        Logger::out(Logger::MaxLevel, "stream2 failed to open\n");
        exit(0);
    }

    float compass_heading_buffer;

    
    while(isRunning)
    {
        cv::Mat image1, image2;
	video1();
	video2();
        printf("entering\n");
        mutex1.lock();
        image1 = buffer1.clone();
        mutex1.unlock();
        mutex2.lock();
        image2 = buffer2.clone();
        mutex2.unlock();
        printf("leaving\n");

        if (image1.data == NULL || image2.data == NULL)
        {
            printf("NULL?!          [%d, %d]\n", image1.data == NULL, image2.data == NULL);
            cv::waitKey(10);
            continue;
        }

        compass_heading_mutex.lock();
        compass_heading_buffer = compass_heading;
        compass_heading_mutex.unlock();
        
        cv::imshow("Feed1", image1);
        cv::imshow("Feed2", image2);
        
        IplImage img1(image1);
        IplImage img2(image2);
        
        IplImage * img_3d = generate3D(&img1, &img2, 1);
        //overlayCompass(img_3d, compass_heading);
        cvShowImage("3D", img_3d);
        IplImage * resizedimg = cvCreateImage( cvSize (1280,960), 8, 3);
        cvResize(img_3d, resizedimg);
        cvShowImage("3D2", resizedimg);
        
        if (cv::waitKey(20) >= 0)
        {
            //isRunning = false;
        }
        
        cvReleaseImage(&img_3d);
        cvReleaseImage(&resizedimg);
    }
	printf("ERROR2\n");
    if (false == stream2.close())
        std::cout << " Warning! stream2 Failed to close properly" << std::endl;
	printf("ERROR2\n");
    if (false == stream1.close())
        std::cout << " Warning! stream1 Failed to close properly" << std::endl;
}

//Called when a key is pressed
void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
        resetFlag = 1;
        break;
    case 27: //Escape key
        exit(0);
    }
}


void keySpecial (int key, int x, int y) {  
    switch (key) {
    case GLUT_KEY_LEFT:
        leftkey = 1;
        break;
    case GLUT_KEY_RIGHT:
        rightkey = 1;
        break;
    case GLUT_KEY_UP:
        upkey = 1;
        break;
    case GLUT_KEY_DOWN:
        downkey = 1;
        break;
    }
} 

void keySpecialUp (int key, int x, int y) {  
    switch (key) {
    case GLUT_KEY_LEFT:
        leftkey = 0;
        break;
    case GLUT_KEY_RIGHT:
        rightkey = 0;
        break;
    case GLUT_KEY_UP:
        upkey = 0;
        break;
    case GLUT_KEY_DOWN:
        downkey = 0;
        break;
    }
} 

//Initializes 3D rendering
void initRendering() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
}

//Called when the window is resized
// edit: left unused
void handleResize(int w, int h) {
    //glViewport(0, 0, w, h);
    //glMatrixMode(GL_PROJECTION);
    //g/lLoadIdentity();
    //gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);
}


void update(int value) { // main program loop
    
    float * angles = gyro.getAngles();
    cout<<angles[0]<<" "<<angles[1]<<" "<<angles[2]<<" "<<upkey<<" "<<downkey<<" "<<leftkey<<" "<<rightkey<<endl;
    float angles2 [6];
    angles2[0] = angles[0];
    angles2[1] = angles[1];
    angles2[2] = angles[2];
    
    if (upkey)
    {
        angles2[3] = 90;
        angles2[4] = -90;
    }
    else if (downkey)
    {
        angles2[3] = -90;
        angles2[4] = 90;
    }
    else if (leftkey)
    {
        angles2[3] = -90;
        angles2[4] = -90;
    }
    else if (rightkey)
    {
        angles2[3] = 90;
        angles2[4] = 90;
    }
    else
    {
        angles2[3] = 0;
        angles2[4] = 0;
    }
    
    if (resetFlag)
    {
        resetFlag = 0;
        gyro.reset_mutex.lock();
        gyro.resetFlag = 1;
        gyro.reset_mutex.unlock();
    }
        
    if (false == socket2.write((const byte*) angles2, 5 * sizeof(float)))
    {
        printf("failed to write the angle!\n");
    }
    if (false == socket2.read((byte*) angles, 3 * sizeof(float)))
    {
        printf("failed to read the angle!\n");

    }
    usleep(50000);
        
        
    compass_heading_mutex.lock();
    compass_heading = angles[0];
    compass_heading_mutex.unlock();

    delete [] angles;
    
    
    
    glutPostRedisplay(); //Tell GLUT that the display has changed
    glutTimerFunc(0, update, 0); // loop this function forever
}

void glEnable2D() // enables 2d drawing
{
    int vPort[4];

    glGetIntegerv(GL_VIEWPORT, vPort);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, vPort[2], 0, vPort[3], -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}
void glDisable2D() // disables 2d drawing
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();   
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();    
}


void draw() { // draws everything
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glutSwapBuffers();
}



int main(int argc, char** argv)
{
    
    if (argc != 3)
    {
        std::cout << " usage: " << argv[0] << " addr1, addr2" << std::endl;
        return -1;
    }
    
    addr1 = argv[1];
    addr2 = argv[2];
    
    printf("Initialize\n");
    

    if (false == socket2.open("172.16.0.2", 8206, Socket::Client, Socket::TCP))
    {
        printf("Failed to open!\n");
        return -1;
    }
    else
        printf("socket open\n");

    

    Logger::setLevel(100);
    Logger::out(99, "Will not print!\n");
    Logger::err(101, "Will print!\n");
    Logger::out(Logger::MaxLevel, "This one will definitely print!\n");
    //boost::thread (boost::bind(&video1));
    //boost::thread (boost::bind(&video2));
    boost::thread(boost::bind(&thread_video));

    // initializations:
    glutInit(&argc, argv);
    //Initialize GLUT
    //glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 600);
    
    //Create the window
    glutCreateWindow("");
    //initRendering();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable2D();
    //Set handler functions
    glutDisplayFunc(draw);
    glutKeyboardFunc(handleKeypress);
    glutSpecialFunc(keySpecial);
    glutSpecialUpFunc(keySpecialUp);
    glutReshapeFunc(handleResize);
    glutTimerFunc(1, update, 0); //Add a timer
    
    // run main program loop
    glutMainLoop();
    isRunning = false;

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

