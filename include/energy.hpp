#pragma once
#include <string>
#include <memory>

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include "openvino/openvino.hpp"

#include <Eigen/Core>
#include <Eigen/Dense>

#include "json.hpp"
#include "debug.hpp"

namespace swq
{
    class GetEnergyMac
    {
    public:
        GetEnergyMac();
        GetEnergyMac(int input_debug, int input_color, int input_mode);
        ~GetEnergyMac();

        void set(int input_debug, int input_color, int input_mode);
        void load_json();
        std::vector<long long int> process(cv::Mat &input_frame);

    private:
        // openvino的初始化
        void openvino_init();
        //获取模型参数
        void model_para_init();
        //将cv::Mat转换为ov::Tensor,包括图像相关的前处理,仅支持FP32精度
        void trans_mat_to_tensor();
        //将模型输出的tensor转换为Matrix,并按照对应形状组织起来,仅支持FP32精度
        void trans_tansor_to_matrix(std::vector<ov::Tensor> out_tenosr);
        //sigmoid函数
        float sigmoid(float input_num);

        //类的状态标志位
        int debug = 0;
        int mode = 0;
        int color = 0;
        //图像中心像素索引
        int img_xCenter;
        int img_yCenter;
        //处理的图像
        cv::Mat frame;
        cv::Mat mask;
        //编译好,已加载到设备的模型
        ov::CompiledModel compiled_model;
        //模型推演请求
        ov::InferRequest infer_request;
        //根据模型结构定义的输入输出管道,
        ov::Output<const ov::Node> input_port;
        ov::Output<const ov::Node> output_port_stride8;
        ov::Output<const ov::Node> output_port_stride16;
        ov::Output<const ov::Node> output_port_stride32;
        //输入模型的向量
        ov::Tensor input_tensor;
        //模型输出，用Eigen的动态数组存储
        Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> output_res;
        //标志位，用于标记数组存储数据的索引
        int res_label = 0;
        //存储目标的空间坐标
        struct armor_final
        {
            long long int x;
            long long int y;
            long long int z;
        };
        //能量机关需要的参数
        struct energy_para
        {
            int center_dis_x;
            int center_dis_y;
            int pass_number_max;
            int delta_angle_distance;
            int frame_size;
            double predict_small;
            double predict_big;
            double R_noise;
            double Q_noise;
            double fan_armor_distence_max;
            double fan_armor_distence_min;
            double armor_R_distance_max;
            double armor_R_distance_min;
            double nms_distence_max;
            std::string model_path;
        };
        //模型相关的参数
        struct out_shape
        {
            int n;
            int c;
            int w;
            int h;
        };
        struct model_para
        {
            int input_n;
            int input_c;
            int input_h;
            int input_w;
            std::string type_str;
            out_shape stride8;
            out_shape stride16;
            out_shape stride32;
        };
        armor_final armor;
        energy_para energy_par;
        model_para model_par;
        
        //模型的anchor直接写死在这里了,如果需要更改训练程序中的anchor，请配合更改这里
        const float anchors[3][6] = {{4,5,  8,10,  13,16},{23,29,  43,55,  73,105},{146,217,  231,300,  335,433}};
        //特征图尺寸
        const float stride[3] = {8.0, 16.0, 32.0};
        //模型类别
        const int classes = 3;

#ifdef COMPILE_DEBUG
    public:
        // debug下用于返回图像
        std::list<cv::Mat> debug_frame();

    private:
        // debug下更新筛选参数
        void updata_argument();
        // debug下json文件里的参数
        void update_json();
#endif
    };
}