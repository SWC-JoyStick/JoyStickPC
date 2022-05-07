  #ifndef RENDER_H
#define RENDER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class Render: public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit Render(QWidget *parent = nullptr);
    virtual ~Render();

    // QWidget虚函数, 用于布局
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setFrameSize(const QSize &frameSize);            // 设置视频帧尺寸, 用于生成纹理
    const QSize &getFrameSize();                          // 获取视频帧尺寸
    void updateTextures(quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 lineSizeY, quint32 lineSizeU, quint32 lineSizeV); // 渲染生成帧

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

private:
    void initShader();             // 初始化着色器
    void initTextures();           // 初始化纹理
    void destroyTextures();        // 清除纹理
    void updateTexture(GLuint texture, quint32 textureType, quint8 *pixels, quint32 stride);       // 更新纹理

    QSize frameSize = {-1, -1};     // 视频帧尺寸
    bool needToUpdate = false;      // 是否需要更新 --- 手机横屏等
    bool textureInited = false;     // 纹理是否已经初始化
    QOpenGLBuffer vbo;              // 顶点缓冲对象(VBO): 默认为VertexBuffer(GL_ARRAT_BUFFER)类型
    QOpenGLShaderProgram shaderProgram;     // 着色器程序: 用于链接着色器
    GLuint texture[3] = {0};        // YUV纹理, 用于生成纹理贴图, 保存3张纹理的id
};

#endif // RENDER_H
