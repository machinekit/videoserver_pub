#include <iostream>
#include <chrono>
#include <sys/time.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <zmq.hpp>

#include "proto/package.pb.h"

using namespace cv;
using namespace std;
namespace po = boost::program_options;

void setupInfoFilter()
{
  namespace logging = boost::log;
  logging::core::get()->set_filter(logging::trivial::severity >=
                                   logging::trivial::info);
}

int main(int argc, char* argv[])
{
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "device,id", po::value<int>()->default_value(0), "Video Device ID")(
      "debug,d", po::value<bool>()->default_value(false),
      "Start in debug mode")("quality,q", po::value<int>()->default_value(90),
                             "JPEG compression level of the image 1-100")(
      "width,w", po::value<int>()->default_value(640), "Width of the frames")(
      "height,h", po::value<int>()->default_value(480), "Height of the frames")(
      "framerate,f", po::value<int>()->default_value(30),
      "The frame rate")("bufferSize,b", po::value<int>()->default_value(3),
                        "Wait for n frame before sending")(
      "uri,u", po::value<string>()->default_value("tcp://127.0.0.1:5563"),
      "ZMQ publish URI");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  int deviceId = vm["device"].as<int>();
  int quality = vm["quality"].as<int>();
  int width = vm["width"].as<int>();
  int height = vm["height"].as<int>();
  int framerate = vm["framerate"].as<int>();
  int bufferSize = vm["bufferSize"].as<int>();
  std::string uri = vm["uri"].as<string>();

  if (vm.count("help"))
  {
    cout << desc << endl;
    return 1;
  }

  if (!vm["debug"].as<bool>())
  {
    setupInfoFilter();
  }

  zmq::context_t context(1);
  zmq::socket_t publisher(context, ZMQ_PUB);
  publisher.bind(uri);
  publisher.setsockopt(ZMQ_LINGER, 0);

  BOOST_LOG_TRIVIAL(debug) << "Built with OpenCV " << CV_VERSION << endl;
  Mat image;

  BOOST_LOG_TRIVIAL(debug) << "Opening video device ID " << deviceId << endl;
  BOOST_LOG_TRIVIAL(debug) << width << "x" << height << " " << framerate
                           << " fps " << endl;
  BOOST_LOG_TRIVIAL(debug) << "quality " << quality << endl;
  VideoCapture capture;
  capture.set(CAP_PROP_FPS, framerate);
  capture.set(CAP_PROP_FRAME_WIDTH, width);
  capture.set(CAP_PROP_FRAME_HEIGHT, height);
  capture.open(deviceId);

  if (!capture.isOpened())
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to open device " << deviceId << endl;
    return -1;
  }
  BOOST_LOG_TRIVIAL(info) << "Capture is opened" << endl;

  pb::Package package;
  std::vector<uchar> outbuf;
  const auto params = std::vector<int>({ IMWRITE_JPEG_QUALITY, quality });
  const std::string TOPIC = "frames";

  for (;;)
  {
    package.Clear();

    for (int i = 0; i < bufferSize; ++i)
    {
      capture >> image;
      if (image.empty())
      {
        BOOST_LOG_TRIVIAL(error) << "Error reading image" << endl;
        return -1;
      }
      imencode(".jpg", image, outbuf, params);
      auto frame = package.add_frame();
      frame->set_blob(outbuf.data(), outbuf.size());
      frame->set_timestamp_unix(time(nullptr));
      struct timeval now
      {
      };
      gettimeofday(&now, nullptr);
      frame->set_timestamp_s(now.tv_sec);
      frame->set_timestamp_us(now.tv_usec);
    }

    const auto string = package.SerializeAsString();

    try
    {
      zmq::message_t topic(TOPIC.data(), TOPIC.size());
      publisher.send(topic, ZMQ_SNDMORE);
      zmq::message_t message(string.data(), string.size());
      publisher.send(message);
    }
    catch (zmq::error_t& e)
    {
      BOOST_LOG_TRIVIAL(error) << e.what() << std::endl;
    }
  }

  return 0;
}
