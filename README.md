# PhysXRollingBall

# 注意：
1. 项目属性配置了一些额外目录，需要在本地环境变量的系统变量中加入如下两条配置：
   变量名：PXSDK_DIR    变量值：你的PhysX SDK目录，比如我的是 D:\PhysX-3.4\PhysX_3.4\
   变量名：PXSDK_BIN    变量值：%PXSDK_DIR%Bin\vc15win64

2. 运行生成exe文件后，若提示找不到某些dll文件，则需要将ExternalDlls文件夹下的dll文件复制到生成exe文件的文件夹下