# xiaoai_linux

   基于小米小爱开案平台Linux SDK 1.1.5  x86_64 lib。

   开发和验证环境：ubuntu 16.04 server(x86_64/bananapi arm32)

   支持 录音+asr识别+nlp+tts.

   举例：启动此示例后说"播放新闻" (或者播放电台/音乐)，则会自动播放当天新闻。

# depends

  asound

  ffmpeg 3.4 (libavcodec libavformat libavutil libavresample)

  SDL2    libSDL2


### FFmpeg compiler param

```

./configure --enable-version3 --disable-x86asm --enable-shared --disable-postproc --disable-swscale --disable-avfilter
         --disable-swscale --disable-avdevice --disable-filters --disable-iconv  --enable-openssl --enable-avresample

  debug param: --enable-debug=3 --disable-stripping

  openssl 为支持https 播放资源

  zlib(zip)为支持http压缩

```
### 

# STEPS

1、[小爱开放平台](https://xiaoai.mi.com/voiceservice/index)注册开放者

2、注册语音产品,设备接入，不控制智能家居设备。授权选择设备Token鉴权

3、下载设备token鉴权的证书。替换deviceTokenAuth目录证书。

4、更新deviceTokenAuth目录profile.py中appid和设备ID等。

5、getRequestSign.sh 获取当前证书的access_token 更新到signal_trigger.c替换appToken参数。

   token有效期7天。当前profile.py和id仅作为示例。可以做测试使用。

6、编译执行

'''
  
 make -f Makefile.full

 //root用户

 ./run.sh

 // 信号控制开启录音

 pkill -10 signal_trigger


'''

# TODO
 1、nlp 支持2.2了

 2、设备Token鉴权 token用的python 刷新整合到程序里面
 

# Reference

  https://xiaoai.mi.com/voiceservice/index

  独立播放器方式
  https://github.com/fenggui321/simple_audio_player

  开发者交流群号： 493191786 

 ![交流群](./doc/qq.png)
