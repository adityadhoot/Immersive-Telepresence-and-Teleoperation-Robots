/**
 * \file VideoStream.cpp
 * \author Billy Jun
 * \brief VideoStream implementation
 */
#include <iostream>
#include <exception>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "VideoStream.h"

/// Default constructor
VideoStream::VideoStream()
{
    // Encapsulate inside a try block to catch any exceptions
    try
    {
        m_stream = (void*) (new cv::VideoCapture);
    }
    catch (...)
    {
        m_stream = NULL;
    }
}

/// Default destructor
VideoStream::~VideoStream()
{
    // Encapsulate inside a try block to catch any exceptions
    try
    {
        if (m_stream)
            delete (cv::VideoCapture*) m_stream;
    }
    catch (...)
    {
    }
}

/**
 * \brief The IP address and port number have to agree with the server side
 * \param[in] addr video stream address
 * \post m_stream stores a pointer to a stream object
 * \return false on failure
 */
bool VideoStream::open(const char* addr)
{
    // Make certain that the pointer is valid
    if (m_stream == NULL)
        return false;

    cv::VideoCapture& cap = *(cv::VideoCapture*) m_stream;
    return cap.open(addr);
}

/**
 * \pre m_stream should not be NULL
 * \post The handle associated with m_stream is cleaned up
 * and m_stream becomes NULL
 * \return false on failure
 */
bool VideoStream::close()
{
    // Make certain that the pointer is valid
    if (m_stream == NULL)
        return false;

    cv::VideoCapture& cap = *(cv::VideoCapture*) m_stream;
    cap.release();
    return true;
}

/**
 * \param[out] image Current image from the stream
 * \return false if the connection gets dropped or some other generic
 * failures occur
 */
bool VideoStream::grabFrame(cv::Mat& image)
{
    // Make certain that the pointer is valid
    if (m_stream == NULL)
        return false;

    // Encapsulate inside a try block to catch any exceptions
    try
    {
        // Typecast to a reference format
        cv::VideoCapture& cap = *(cv::VideoCapture*) m_stream;

        // Acquire a frame.
        cap >> image;
    }
    catch (std::exception e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown error while grabbing a frame" << std::endl;
        return false;
    }

    return true;
}
