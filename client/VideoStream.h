/**
 * \file VideoStream.h
 * \author Billy Jun
 * \brief VideoStream declaration
 */
#pragma once

#include <opencv2/core/core.hpp>

class VideoStream
{
  public:
    /// Default constructor
    VideoStream();
    /// Default destructor
    ~VideoStream();

    /**
     * \brief The IP address and port number have to agree with the server side
     * \param[in] addr video stream address
     * \post m_stream stores a pointer to a stream object
     * \return false on failure
     */
    bool open(const char* addr);

    /**
     * \pre m_stream should not be NULL
     * \post The handle associated with m_stream is cleaned up
     * and m_stream becomes NULL
     * \return false on failure
     */
    bool close();

    /**
     * \param[out] image Current image from the stream
     * \return false if the connection gets dropped or some other generic
     * failures occur
     */
    bool grabFrame(cv::Mat& image);

  private:
    void* m_stream;
};
