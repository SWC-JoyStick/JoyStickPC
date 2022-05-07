#ifndef FRAMES_H
#define FRAMES_H

#include <QMutex>
#include <QCoreApplication>
#include <QWaitCondition>

typedef struct AVFrame AVFrame;

class Frames {
public:
    Frames();
    virtual ~Frames();

    bool init();                                    // 初始化frame, 给frame分配空间
    void destroy();                                 // 释放frame空间
    void lock();                                    // 互斥锁, 保证AVFrame线程安全
    void unlock();
    AVFrame *getDecodingFrame();                    // 返回正在解码的帧
    bool offerDecodedFrame();                       // 交换解码/渲染帧, 返回是否已经使用了之前渲染的帧
    const AVFrame *consumeRenderedFrame();          // 使用当前渲染完的帧, 即将该帧送到显示器显示
    void stop();                                    // 停机

private:
    void swap();                                    // 交换编码帧和渲染帧
    /* 此处解码帧和渲染帧采用了双缓冲/流水线思想
     * 渲染当前帧的同时解码下一帧, 渲染完成同时解码完成
     * 交换渲染帧和解码帧, 继续解码和渲染新帧
     */
    AVFrame *decodingFrame = Q_NULLPTR;             // 保存正在解码的一帧YUV数据
    AVFrame *renderingFrame = Q_NULLPTR;            // 保存正在渲染的一帧YUV数据
    QMutex mutex;                                   // 保证AVFrame的线程安全
    QWaitCondition renderingFrameConsumedCond;      // 控制当前渲染帧的条件变量
    bool isRenderingFrameConsumed = true;           // 当前渲染帧是否已经完成: 初始无渲染帧, 因此为true
    bool renderExpiredFrames = false;               // 是否渲染过期帧
    bool interrupted = false;                       // 是否中断
};

#endif // FRAMES_H
