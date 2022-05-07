#include <QCoreApplication>
#include <QOpenGLTexture>
#include "render.h"

// 存储顶点坐标和纹理坐标, 一起存在vbo中
static const GLfloat coordinate[] = {
    // 顶点坐标, 存储4个xyz坐标
    // GL_TRIANGLE_STRIP绘制方式
    // x    y    z
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    // 纹理坐标, 存储4个xy坐标
    // 左下角为[0, 0]
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f
};

// vertex shader
static const QString vertexShader = R"(
    attribute vec3 vertexIn;           // xyz顶点坐标
    attribute vec2 textureIn;          // xy纹理坐标
    varying vec2 textureOut;        // 传递给fragment shader的纹理坐标
    // out vec4 VertexColor
    void main(void) {
        gl_Position = vec4(vertexIn, 1.0);      // 指定顶点绘制位置
        textureOut = textureIn;                 // 纹理坐标直接传递给fragment shader
        // VertexColor = gl_Color;
    }
)";

// fragment shader
static QString fragShader = R"(
    varying vec2 textureOut;         // vertex shader传过来的纹理坐标
    // in vec4 VertexColor;
    uniform sampler2D textureY;
    uniform sampler2D textureU;
    uniform sampler2D textureV;
    void main(void) {
        vec3 yuv;
        vec3 rgb;

        // from SDL2
        const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);
        const vec3 Rcoeff = vec3(1.1644,  0.000,  1.7927);
        const vec3 Gcoeff = vec3(1.1644, -0.2132, -0.5329);
        const vec3 Bcoeff = vec3(1.1644,  2.1124,  0.000);
        // yuv采样
        yuv.x = texture2D(textureY, textureOut).r;
        yuv.y = texture2D(textureU, textureOut).r;
        yuv.z = texture2D(textureV, textureOut).r;
        yuv += offset;
        // 转化rgb
        rgb.r = dot(yuv, Rcoeff);
        rgb.g = dot(yuv, Gcoeff);
        rgb.b = dot(yuv, Bcoeff);
        // 输出颜色值
        gl_FragColor = vec4(rgb, 1.0);
        // gl_FragColor = vec4(rgb, 1.0) * VertexColor;
    }
)";


Render::Render(QWidget *parent): QOpenGLWidget(parent) {

}

Render::~Render() {
    makeCurrent();
    vbo.destroy();          // 销毁VBO
    destroyTextures();     // 销毁纹理
    doneCurrent();
}

QSize Render::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize Render::sizeHint() const {
    return size();
}

void Render::setFrameSize(const QSize &frameSize) {
    if (this->frameSize != frameSize) {
        this->frameSize = frameSize;
        needToUpdate = true;
        repaint();          // 调用paintGL
    }
}

const QSize &Render::getFrameSize() {
    return frameSize;
}

void Render::updateTextures(quint8 *dataY, quint8 *dataU, quint8 *dataV, quint32 lineSizeY, quint32 lineSizeU, quint32 lineSizeV) {
    if (textureInited) {
        // 使用渲染后的yuv帧更新yuv纹理
        updateTexture(texture[0], 0, dataY, lineSizeY);
        updateTexture(texture[1], 1, dataU, lineSizeU);
        updateTexture(texture[2], 2, dataV, lineSizeV);
        // 更新ui
        update();
    }
}

void Render::initializeGL() {
    initializeOpenGLFunctions();
    glDisable(GL_DEPTH_TEST);           // 关闭深度测试, 没有用到
    // 创建VBO并绑定
    vbo.create();
    vbo.bind();
    vbo.allocate(coordinate, sizeof(coordinate));
    // 初始化shader
    initShader();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Render::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    repaint();
}

void Render::paintGL() {
    if (needToUpdate) {
        destroyTextures();
        initTextures();
        needToUpdate = false;
    }

    if (textureInited) {
        for (int i = 0; i < 3; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texture[i]);
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

void Render::initShader() {
    if (QCoreApplication::testAttribute(Qt::AA_UseOpenGLES)) {      // 如果使用opengles
        // 手动指定opengles的int和float精度
        fragShader.prepend(R"(
        precision mediump int;
        precision mediump float;
        )");
    }

    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragShader);
    shaderProgram.link();
    shaderProgram.bind();
    // 指定顶点坐标在VBO的访问方式
    // shader: { "name": , 坐标类型: GL_FLOAT, 起始偏移量: 0, 顶点坐标个数: 3, 步幅: 3 * sizeof(float) }
    shaderProgram.setAttributeBuffer("vertexIn", GL_FLOAT, 0, 3, 3 * sizeof(float));
    shaderProgram.enableAttributeArray("vertexIn");     // 启动顶点属性

    // 指定纹理坐标在VBO的访问方式
    shaderProgram.setAttributeBuffer("textureIn", GL_FLOAT, 12 * sizeof(float), 2, 2 * sizeof(float));
    shaderProgram.enableAttributeArray("textureIn");

    // 关联fragShader和opengl纹理单元
    shaderProgram.setUniformValue("textureY", 0);
    shaderProgram.setUniformValue("textureU", 1);
    shaderProgram.setUniformValue("textureV", 2);
}

void Render::initTextures() {
    for (int i = 0; i < 3; i++) {
        QSize size = (i == 0) ? frameSize : frameSize / 2;
        // 创建纹理
        glGenTextures(1, &texture[i]);
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        // 设置纹理缩放时的策略
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       // 线性过滤策略
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 设置st方向上纹理超出坐标时的显示策略
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, size.width(), size.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);
    }
    // 标志位
    textureInited = true;
}

void Render::destroyTextures() {
    if (QOpenGLFunctions::isInitialized(QOpenGLFunctions::d_ptr))
        glDeleteTextures(3, texture);

    memset(texture, 0, sizeof(texture));
    textureInited = false;
}

void Render::updateTexture(GLuint texture, quint32 textureType, quint8 *pixels, quint32 stride) {
    if (!pixels)
        return;

    QSize size = (textureType == 0) ? frameSize : frameSize / 2;
    makeCurrent();
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, static_cast<GLint>(stride));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.width(), size.height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
    doneCurrent();
}
