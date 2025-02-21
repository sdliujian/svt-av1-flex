SVT- AV1 源码结构：
==============================

SVT-AV1 源码主要包含如下目录和文件： 

1.  **Source**
   - 包含 SVT-AV1 encoder 和 decoder 的源码
   - 子目录:
     - `Lib`:  核心库和模块.
     - `API`: 一些头文件，包含对外接口.
     - `Encoder`:  编码器.
     - `Decoder`:  解码器.

2. **Build**
   - 包含构建源代码的脚本和配置文件.
   - 子目录:
     - `CMakeLists.txt`.
     - `Makefile`.

3. **Test**
   - 包含测试用例代码.
   - 子目录:
     - `Functional`: 功能测试.
     - `Performance`: 性能测试和基准.
     - `Unit`: 单元测试.

4. **Docs**
   - 文档.

5. **Config**
   - 配置文件.

6. **ffmpeg_plugin**
   - 与ffmpeg 集成所需的说明和补丁


———————————————————

1. 源码构建和使用，参考：
     Docs/svt-av1_encoder_user_guide.md

2. SVT-AV1 的设计文档可以参考：
     Docs/svt-av1-encoder-design.md

     其中详细描述了编码的流程，对多数技术都有概要说明

3. 程序入口

    Source/App/EncApp/EbAppMain.c

    可以从这里开始调试，查看编码过程

4.  详细代码主要在：

   Source/Lib/Encoder/Codec/ 

===================================
SVT-AV1 隐藏参数
===================================
SVT-AV1 有一个循环反馈的编码过程，会通过rate-control-process 来控制生成的码率。编码之前可以预设一个目标码率，然后编码器就会朝着这个目标进行优化

所以其中一些控制码率的参数并不是预先设定好的，而是编码过程中自动调整的。

有一些参数和机制可以分别控制视频帧的动态区域和静态区域的质量， 这些参数位于 rate control, quantization 量化,  和 perceptual optimization 等模块中.  我们可以通过阅读 rate control, quantization, 和motion analysis 等模块来找到这些参数.

---

### 1. **Rate Control and Quantization Parameters**
   - **源码位置**:
     - `Source/Lib/Encoder/Codec/EbRateControlProcess.c`
     - `Source/Lib/Encoder/Codec/EbRateControlProcess.h`

   - **关键参数**:
     - **`qp` (Quantization Parameter)**: 控制不同区域的编码质量.  自适应 adaptive quantization (AQ)  可能根据运动或纹理复杂度来调整QP
     - **`aq_mode` (Adaptive Quantization Mode)**: 决定如何对不同区域调整量化参数( 例如, motion vs. non-motion areas).
     - **`vbr` (Variable Bitrate)**:  在 VBR 模式中, 编码器会给剧烈运动的区域分配更多的bit.
   
   - **查找提示**:
     - 搜索一些关键字，例如 `aq_strength`, `aq_mode`,  `adaptive_quantization`.
     - 有关于 motion，spatial activity，QP 的函数

---

### 2. **Motion Analysis 和 Activity Maps**
   - **位置**: Motion analysis 在 motion estimation（运动估计） 和 motion compensation（运动补偿） 模块中实现:
     - `Source/Lib/Encoder/Codec/EbMotionEstimation.c`
     - `Source/Lib/Encoder/Codec/EbMotionCompensation.c`
   
   - **关键参数**:
     - **`motion_threshold`**: 用于区分运动区域和静止区域的阈值.
     - **`activity_map`**: 用于识别高速运动或复杂纹理区域的映射表
      
   - **查找提示**:
     - 搜索一些关键词，例如`motion_threshold`, `activity_map`, or `motion_intensity`.
     - 搜索计算运动向量(motion vector) 的函数.

---

### 3. **视觉感知优化**
   - **源码位置**: Perceptual optimization 通常在于视觉质量调优模块中处理:
     - `Source/Lib/Encoder/Codec/EbPictureAnalysis.c`
     - `Source/Lib/Encoder/Codec/EbPictureControlSet.c`
   
   - **关键参数**:
     - **`tune_metric`**: 控制编码器是否优先保障运动区域或静止区域的质量.
     - **`perceptual_optimization`**: 打开/关闭视觉质量优化.

   - **查找提示**:
     - 搜索一些关键词，例如： `perceptual_optimization`, `tune_metric`, 或者 `visual_quality`.

---

### 4. **帧级别或者块级别的质量控制**
   - **源码位置**:  帧级别或块级别的质量控制经常在"模式选择"或"划分"模块中实现:
     - `Source/Lib/Encoder/Codec/EbModeDecision.c`
     - `Source/Lib/Encoder/Codec/EbPartitioning.c`
   - **关键参数**:
     - **`block_qp_offset`**: 根据运动或纹理为特定的块调整量化参数 QP .
     - **`partitioning_decision`**:  决定像素块应该如何切分，这会影响运动区域和静止区域的图像质量.
   - **查找提示**:
     - 搜索一些关键字，例如 `block_qp_offset`, `partitioning_decision`, 或 `mode_decision`.

---

### 5. **命令行界面(CLI) 参数**
   - **源码位置**: 某些隐藏的或高级的参数可能在CLI中暴露，但是没有卸载使用指南中。查看一下CLI 输入参数分析代码:
     - `Source/App/EncApp/EbAppConfig.c`
   - **查找提示**:
     - 搜索一些关键词，例如 `motion_quality`, `static_quality`, 或者 `adaptive_quality`.

---

### 示例：寻找自适应量化参数
如果要寻找自适应的量化参数 (adaptive quantization parameters), 或许能在 `EbRateControl.c`等文件中找到。
```c
if (rc->aq_mode == AQ_MODE_AUTO) {
    // Adjust QP based on motion or texture activity
    qp_offset = calculate_qp_offset(activity_map, motion_intensity);
}
```
这里, `activity_map` 和 `motion_intensity` 就可能用于控制运动区域和静止区域的质量。

---

一些重要位置：
•   EbRateControlProcess.c:225-288  
•   EbModeDecisionConfigurationProcess.c:45-85  
•   Appendix-Rate-Control.md:580-634  
•   firstpass.h:143-241  
•   Parameters.md:234-249  
•   encoder.h:127-247

控制运动区域和非运动区域码率的参数可以在一些文件中找到，关键参数包括：
	1	EbRateControlProcess.c:
		STATIC_MOTION_THRESH
		ME_SAD_LOW_THRESHOLD1, ME_SAD_LOW_THRESHOLD2, ME_SAD_HIGH_THRESHOLD
		MAX_REF_AREA_I, MAX_REF_AREA_NONI, REF_AREA_LOW_THRESHOLD, REF_AREA_MED_THRESHOLD
		FAST_MOVING_KF_GROUP_THRESH, MEDIUM_MOVING_KF_GROUP_THRESH
	2	firstpass.h:
		FRAME_STATS structure members such as coded_error, mv_count, inter_count, second_ref_count, neutral_count, intra_skip_count, sum_in_vectors, sum_mvc, sum_mvr_abs, sum_mvc_abs
	3	Appendix-Rate-Control.md:
		Descriptions of functions such as process_rc_stat(), av1_set_target_rate(), rc_pick_q_and_bounds()

可以去EbRateControlProcess.c and Appendix-Rate-Control.md 等文件中寻找.
