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

——————————————————————

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
   
   - **查找**:
     - 查找 `aq_strength`, `aq_mode`,  `adaptive_quantization`.
     - 有关于 motion，spatial activity，QP 的函数

---

### 2. **Motion Analysis 和 Activity Maps**
   - **位置**: Motion analysis 在 motion estimation（运动估计） 和 motion compensation（运动补偿） 模块中实现:
     - `Source/Lib/Encoder/Codec/EbMotionEstimation.c`
     - `Source/Lib/Encoder/Codec/EbMotionCompensation.c`
   - **Key Parameters**:
     - **`motion_threshold`**: A threshold to distinguish between motion and non-motion areas.
     - **`activity_map`**: A map that identifies regions with high motion or texture complexity.
   - **What to Look For**:
     - Search for terms like `motion_threshold`, `activity_map`, or `motion_intensity`.
     - Look for functions that calculate motion vectors or analyze motion activity.

---

### 3. **Perceptual Optimization**
   - **Location**: Perceptual optimization is often handled in modules related to visual quality tuning:
     - `Source/Lib/Encoder/Codec/EbPictureAnalysis.c`
     - `Source/Lib/Encoder/Codec/EbPictureControlSet.c`
   - **Key Parameters**:
     - **`tune_metric`**: Controls whether the encoder prioritizes visual quality in motion or static areas.
     - **`perceptual_optimization`**: A flag or parameter that enables/disables perceptual tuning.
   - **What to Look For**:
     - Search for terms like `perceptual_optimization`, `tune_metric`, or `visual_quality`.

---

### 4. **Frame-Level and Block-Level Control**
   - **Location**: Frame-level and block-level quality control is often handled in the mode decision and partitioning modules:
     - `Source/Lib/Encoder/Codec/EbModeDecision.c`
     - `Source/Lib/Encoder/Codec/EbPartitioning.c`
   - **Key Parameters**:
     - **`block_qp_offset`**: Adjusts QP for specific blocks based on motion or texture.
     - **`partitioning_decision`**: Determines how blocks are split, which can affect quality in motion vs. non-motion areas.
   - **What to Look For**:
     - Search for terms like `block_qp_offset`, `partitioning_decision`, or `mode_decision`.

---

### 5. **Command-Line Interface (CLI) Parameters**
   - **Location**: Some hidden or advanced parameters might be exposed in the CLI but not documented in the user guide. Check the CLI parsing code:
     - `Source/App/EncApp/EbAppConfig.c`
   - **What to Look For**:
     - Search for terms like `motion_quality`, `static_quality`, or `adaptive_quality`.

---

### Example: Finding Adaptive Quantization Parameters
If you’re looking for adaptive quantization parameters, you might find code like this in `EbRateControl.c`:
```c
if (rc->aq_mode == AQ_MODE_AUTO) {
    // Adjust QP based on motion or texture activity
    qp_offset = calculate_qp_offset(activity_map, motion_intensity);
}
```
Here, `activity_map` and `motion_intensity` might be used to control quality in motion vs. non-motion areas.

---

### Tips for Exploring the Code
1. **Use `grep` or IDE Search**:
   - Use `grep` or your IDE’s search function to look for keywords like `motion`, `activity`, `qp`, or `quality`.
2. **Follow Function Calls**:
   - Start from the main encoding loop and trace function calls related to rate control, motion estimation, and mode decision.
3. **Experiment with Parameters**:
   - Once you identify relevant parameters, modify them and observe the impact on encoding quality and performance.

If you have a specific parameter or functionality in mind, let me know, and I can help narrow down the search!

	•	EbRateControlProcess.c:225-288  EbModeDecisionConfigurationProcess.c:45-85  Appendix-Rate-Control.md:580-634  firstpass.h:143-241  Parameters.md:234-249  encoder.h:127-247

The parameters that control the code rate for motion areas and non-motion areas in the encoder library can be found in several files. Key parameters include:
	1	EbRateControlProcess.c:
	◦	STATIC_MOTION_THRESH
	◦	ME_SAD_LOW_THRESHOLD1, ME_SAD_LOW_THRESHOLD2, ME_SAD_HIGH_THRESHOLD
	◦	MAX_REF_AREA_I, MAX_REF_AREA_NONI, REF_AREA_LOW_THRESHOLD, REF_AREA_MED_THRESHOLD
	◦	FAST_MOVING_KF_GROUP_THRESH, MEDIUM_MOVING_KF_GROUP_THRESH
	2	firstpass.h:
	◦	FRAME_STATS structure members such as coded_error, mv_count, inter_count, second_ref_count, neutral_count, intra_skip_count, sum_in_vectors, sum_mvc, sum_mvr_abs, sum_mvc_abs
	3	Appendix-Rate-Control.md:
	◦	Descriptions of functions such as process_rc_stat(), av1_set_target_rate(), rc_pick_q_and_bounds()
You can refer to the specific sections in the EbRateControlProcess.c and Appendix-Rate-Control.md files for more details.
