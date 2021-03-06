

#include "UpdateThread.hpp"
#include <unistd.h>
#include <utility>

using namespace robot_remote_control;


UpdateThread::UpdateThread():running(false), threadTimer(std::make_shared<ThreadProtectedVar<Timer>>()) {
}

UpdateThread::~UpdateThread() {
    if (running) {
        stopUpdateThread();
    }
}

void UpdateThread::updateThreadMain(const unsigned int &milliseconds, std::future<void> runningFuture, std::shared_ptr< ThreadProtectedVar<Timer> > timer) {
    running = true;
    while (runningFuture.wait_for(std::chrono::milliseconds(milliseconds)) == std::future_status::timeout) {
        update();
        timer->lock();
        timer->get_ref().start();
        timer->unlock();
    }
    running = false;
}

void UpdateThread::startUpdateThread(const unsigned int &milliseconds) {
    if (!running) {
        stopFuture = stopPromise.get_future();
        updateThread = std::thread(&UpdateThread::updateThreadMain, this, std::move(milliseconds), std::move(stopFuture), threadTimer);
        running = true;
    }
}

void UpdateThread::stopUpdateThread() {
    if (running) {
        stopPromise.set_value();
        if (updateThread.joinable()) {
            updateThread.join();
        }
        running = false;
    }
}

float UpdateThread::getElapsedTimeInS() {
    threadTimer->lock();
    float time = threadTimer->get_ref().getElapsedTime();
    threadTimer->unlock();
    return time;
}
