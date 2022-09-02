
#include "Eigen/Core"
#include <igl/unproject_onto_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/readOFF.h>
#include <igl/readOBJ.h>
// #include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/per_vertex_normals.h>
#include <igl/writeOBJ.h>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cmath>
#include <utility>
#include <chrono>
#include <thread>
#include <vector>
#include "arap.hpp"
#include <json/json.h>
#include <time.h>
#include <mtface/mtface.h>
#include <mtface/types.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

using std::map;
using std::pair;
using igl::opengl::glfw::Viewer;

bool hasEnding (std::string const &fullString, std::string const &ending) {
    
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

FixedVertex closest_point(Eigen::Vector2f mouse,
			   const Mesh& mesh, const std::vector<FixedVertex>& group,
			   Eigen::Matrix4f view, Eigen::Matrix4f proj,
			   Eigen::Vector2f point_size) {
    float closest = -1;
    FixedVertex chosen = {-1, 0};

    float threshold = point_size.squaredNorm();

    Eigen::Vector4f p;
    p(3) = 1;
    for (int i = 0; i < group.size(); i++) {
	p.block<3, 1>(0, 0) = mesh.V.row(group[i].index).cast<float>();  // group[i].index:我们选定的关键点下标值
	// std::cout<<"group:"<<group[i].index<<"\n";
	Eigen::Vector4f projected = (proj * view * p);
	projected /= projected(3);

	// Eigen::Vector2f projected_pixels = (projected.block<2, 1>(0, 0).array() + 1.) * viewport.block<2, 1>(2, 0).array() / 2.;
	
	if ((projected.block<2, 1>(0, 0) - mouse).squaredNorm() <= threshold
	    && (closest < 0 || projected(2) < closest)) {
	    closest = projected(2);
	    chosen = group[i];
	}
    }

    return chosen;
}

void update_group(const Eigen::MatrixXd& V, const std::vector<FixedVertex>& group, Eigen::MatrixXd& group_pos) {
    for (int i = 0; i < group.size(); i++) {
	group_pos.row(i) = V.row(group[i].index);
    }
}

Eigen::Vector2f mouse_position(const Viewer& viewer) {
    Eigen::Vector2f dimensions = viewer.core().viewport.block<2, 1>(2, 0);
    Eigen::Vector2f mouse_pos(
		viewer.current_mouse_x,
		viewer.core().viewport(3) - viewer.current_mouse_y
	);

    mouse_pos.array() = 2. * mouse_pos.array() / dimensions.array() - 1.;

    return mouse_pos;
}

Eigen::Vector4f unproject_mouse(const Viewer& viewer, Eigen::Vector3f point) {
    Eigen::Vector2f mouse_pos = mouse_position(viewer);
    Eigen::Matrix4f viewproj = viewer.core().proj * viewer.core().view;  // 得到一个变换矩阵

    Eigen::Vector4f point_homo;
    
    point_homo.block<3, 1>(0, 0) = point.cast<float>();
    point_homo(3) = 1.;
    Eigen::Vector4f projected_sel = viewproj * point_homo;   //变换的只有物体，矩阵乘法：44x41=41，计算Z轴
    projected_sel /= projected_sel(3);

    Eigen::Vector4f mouse_homo(mouse_pos(0), mouse_pos(1), projected_sel(2), 1.);
    Eigen::Vector4f unprojected_mouse = viewproj.inverse() * mouse_homo;
    unprojected_mouse /= unprojected_mouse(3);

    return unprojected_mouse;
}

bool compare_by_index(const FixedVertex& v1, const FixedVertex& v2) {
    return v1.index < v2.index;
}

Eigen::Vector3d group_color(size_t g) {
    Eigen::Vector3d color;
    switch(g % 6) {
    case 0:
	color = Eigen::Vector3d(26, 153, 136);
	break;
    case 1:
	color = Eigen::Vector3d(235,86,0);
	break;
    case 2:
	color = Eigen::Vector3d(183, 36, 92);
	break;
    case 3:
	color = Eigen::Vector3d(243,183,0);
	break;
    case 4:
	color = Eigen::Vector3d(55,50,62);
	break;
    case 5:
	color = Eigen::Vector3d(52,89,149);
	break;
    }

    Eigen::Vector3d white(255, 255, 255);
    for (int i = 0; i < g / 6; i++) {
	color = white - (white - color) * .75;
    }
    return color / 255;
}

bool load_model(const std::string model_name, Mesh& mesh) {
    if (hasEnding(model_name, ".off")) {
        std::cout<<"argv[1]1:"<<model_name<<std::endl;
	    igl::readOFF(model_name, mesh.V, mesh.F);
    } else if (hasEnding(model_name, ".obj")) {
	    igl::readOBJ(model_name, mesh.V, mesh.F);
    } else {
	    return false;
    }

    igl::per_vertex_normals(mesh.V, mesh.F, mesh.N);

    return true;
}
void solve_loop(LaplacianSystem* system) {
    while (true) {
	system_iterate(*system);
    }
}
void benchmark(const std::string& model_name, int iterations) {
    using namespace std::chrono;
    Mesh mesh;
    LaplacianSystem system;

    std::vector<FixedVertex> fixed_vertices = {
	{2833, 0},
	{3161, 0},
	{1589, 0},
	{3153, 1},
    };
    
    if (!load_model(model_name, mesh)) {
	return;
    }

    system_init(system, &mesh, 0.);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
	std::cout<<"final——---index:------:"<<system.mesh->V.row(fixed_vertices[2].index) <<"\n";
    for (const auto& vertex : fixed_vertices) {
        if (vertex.index!=fixed_vertices[2].index){
        std::cout<<"--------------index:------:"<<vertex.index <<"\n";
        std::cout<<"before vertex.index:------:"<<system.mesh->V.row(vertex.index) <<"\n";
        system.mesh->V.row(vertex.index) = system.mesh->V.row(fixed_vertices[2].index);
        std::cout<<"after  vertex.index:------:"<<system.mesh->V.row(vertex.index) <<"\n";
		// system_solve(system, iterations);
		
        }
				
	}
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
	// std::thread solver_thread(solve_loop, &system);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);

    std::cout << model_name << ", " << mesh.V.rows() << ", " << elapsed.count() / iterations << std::endl;
	igl::writeOBJ("mesh_deformation.obj",system.mesh->V,system.mesh->F);
	std::cout<<mesh.V.rows()<<std::endl;
}

void vertic_move(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }

    for (const auto& vertex : fixed_vertices) {
        system.mesh->V.row(vertex.index) = system.mesh->V.row(int(vertex.group));

        // if (vertex.index!=fixvec){
        //     // std::cout<<"vertex.index:"<<vertex.index<<"  fixed_vertices[0].index:"<<system.mesh->V.row(fixed_vertices[0].index)<<std::endl;
        //     system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec);
        // }		
	}
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save mesh"<<std::endl;
    igl::writeOBJ("mesh_deformation.obj",system.mesh->V,system.mesh->F);
	std::cout<<mesh.V.rows()<<std::endl;  
}

void Face_Contour_Restoration(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations, int fu) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }

    for (const auto& vertex : fixed_vertices) {

        // std::cout<<"vertex.index:"<<vertex.index<<"  fixed_vertices[0].index:"<<system.mesh->V.row(fixed_vertices[0].index)<<std::endl;
        // double time = (1 + (double(vertex.group)/(double)550));
        // double time = (1 + (double(vertex.group)/double(vertex.group + vertex.group * 0.02)));
        
        float time = vertex.group;

        // if (time < 1.07){
        //     time = 1.;
        // }
        
        if (fu==0 && time >0){
            float futime = time + 0.12;
            std::cout<<"1fu:"<<fu<<" time= "<<vertex.group<<" index:"<<vertex.index<<std::endl;
            time = time + 2.;
            
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[0] * futime;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * futime;	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime;
            
            // system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] * time;
            // system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] * time;	
            // system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] * time;
            
        
        }else if(time > 0){
            time = time + 1;
            std::cout<<"2fu:"<<fu<<" time="<<time<<std::endl;
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] * time;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] * time;	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] * time;
        }
    }
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save mesh"<<std::endl;
    igl::writeOBJ("face_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	std::cout<<mesh.V.rows()<<std::endl;  
}
// //---------------------------------------------------------------调用人脸库code----------------------------------------
// std::string CMAKE_SOURCE_DIR = "/Users/wpx/Documents/code/arap/libmtface/macos";
void onError(char *error)
{
    std::cout << error << std::endl;
    mtface_free(error);
    abort();
}

class ThrowOnError
{
public:
    ~ThrowOnError()
    {
        expect();
    }

    void expect()
    {
        if (err_)
        {
            std::cout << err_ << std::endl;
            auto e = std::runtime_error(err_);
            mtface_free(err_);
            err_ = nullptr;
            throw e;
        }
    }

    operator char **()
    {
        expect();
        return &err_;
    }

private:
    char *err_ = nullptr;
};

//--------------mesh和keypoint计算最近距离--------------------------------
typedef pair<int, int> PAIR;
struct CmpByValue {
  bool operator()(const PAIR& lhs, const PAIR& rhs) {
    return lhs.second < rhs.second;
  }
};
Eigen::MatrixX3f computer_distance(Eigen::MatrixX2f &key_point, Eigen::MatrixX2f &mesh_vet){
    float dis;
    Eigen::Matrix<float, Eigen::Dynamic, 3> di;
    Eigen::Matrix<float, 1,2> mesh;
    Eigen::Matrix<float, 1,2> key;
    int key_count = 0;
    Eigen::MatrixX2f::Index maxRow, maxCol;
    maxRow = mesh_vet.rows();
    maxCol = 0;
    di.resize(key_point.rows(), 3);
    double mids = 0;
    double sigma = 0;
    std::cout<<"key_point:"<<key_point.rows()<<std::endl;
    for (int i=0; i< key_point.rows(); i++){
        Eigen::Matrix<float, Eigen::Dynamic, 1> item;
        item.resize(mesh_vet.rows(), 1);
        key= key_point.row(i);
        for (int count=0; count<mesh_vet.rows(); count++){
            mesh = mesh_vet.row(count);
            dis = sqrt(pow((key[0] - mesh[0]),2) + pow((key[1] - mesh[1]), 2));
            item(count, 0) = dis;
        }
        float min = item.col(0).minCoeff(&maxRow,&maxCol);
        for(int j=0; j<item.rows(); j++){
            if (min == item.row(j)[0]){
                di(key_count, 0) = key_count;  // FA点
                di(key_count, 1) = j;   // 距离FA点最近的mesh点
                di(key_count, 2) = min;  // FA与最近的mesh点距离是多少。
                
                
                if (key_count< 33){
                    // std::cout<<key_count<<": "<<min<<std::endl;
                    mids += min;
                }
                key_count++;
                break;
            }
        }   
    };
    double means;
    means = mids / double(33);
    for (int i=0; i< 33; i++){
        sigma += pow((di.row(i)[2] - means), 2);
    }
    sigma = sigma / double(33);
    for(int i =0; i< 33; i++){
        // std::cout<<(di.row(i)[2] - means) / sigma<<"\n";
        di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
    }
    std::cout<<"sigma: "<<sigma<<" means: "<<mids / double(33)<<std::endl;
    return di;
};
//--------------mesh和keypoint计算最近距离--------------------------------

Eigen::MatrixX2f readFileJson(std::string json_path, std::string filename)
{
    Json::Reader reader;
    Json::Value root;

    std::ifstream in(json_path,std::ios::binary);
    Eigen::Matrix<float, Eigen::Dynamic, 2> keypoint;
    Eigen::Vector2f vec;
    if(!in.is_open()){
        std::cout<<"Error opening"<<std::endl;
        return keypoint;
    }
    
    if(reader.parse(in,root)){

        //读取子节点--字典
        
        const Json::Value partner = root["humans"];
        // std::cout<<"partner:"<<partner.size()<<std::endl;
        for(unsigned int i=0;i<partner.size();i++){
            const Json::Value items = partner[0][filename];  // 找到face keypoint 对应的2D点
            // const Json::Value items = partner[0]["mesh"];   // 找到 mesh 对应的 2D点
            std::cout<<"items:"<<items.size()<<std::endl;
            keypoint.resize(items.size(), 2);
            for (unsigned int j=0;j<items.size(); j++){
                keypoint(j,0) = items[j][0].asFloat();
                keypoint(j,1) = items[j][1].asFloat();    
            }
        }
    }

    in.close();
    return keypoint;
}


void writeFileJson(const Eigen::MatrixXf& point, std::string jsonfile, std::string two_title_name)  
{ 
    //根节点  
    Json::Value root;   // 创建一个一级节点
    Json::Value second; // 创建一个二级节点
    
    for(int i=0;i<point.rows(); i++){
        float a[2] = {point.row(i)[0], point.row(i)[1]};
        Json::Value third;  // 因为我们要实现数组的追加，所以我们需要在这一层创建一个三级节点
        for(auto i : a) {
            third.append(i);  // 因为对于C++ Json来说，只支持单个字符的追加，所以我们这里需要这句代码；
        }
        second[two_title_name].append(third);  // 追加完之后，将追加的数组放在上一个数组之后。
    }
    //根节点属性  
    root["humans"].append(second);  // 将我们的二级节点与一节节点关联起来
    Json::StyledWriter sw;    // 创建一个带有格式的写入
    Json::FastWriter fw; 
    // std::ofstream desFile(jsonfile, std::ios::out | std::ios::app);  // 不删除之前内容的添加
    std::ofstream desFile(jsonfile);  // 删除之前内容的添加
	if (!desFile.is_open()) // 判断是否可以打开json文件；
	{
		std::cout << "Fail to pen des.jons";
	}
	desFile << fw.write(root);  // 不带有格式的写入到json文件中。（就是在一行上写入所有内容）
    // desFile << sw.write(root);  // 有格式的写入到json文件中。（多行显示）
	desFile.close();  // 记得关闭打开的json文件
}


// 将3Dmesh 生成 2D坐标
bool ThreeDimTo2Dim(cv::Mat& image, Eigen::Map<const Eigen::Matrix3Xf>& mesh, Eigen::Matrix4f& mvp,  Eigen::Map<const Eigen::Matrix<uint16_t, 3, Eigen::Dynamic>>& trig, std::string jsonfile, std::string mesh_obj_path){  
    int image_width, image_height;
    std::fstream f;
    image_width = image.rows;
    image_height = image.cols;
    Eigen::Matrix4f viewport;
    viewport << (image_width - 1) / 2.0, 0.0, 0.0, (image_width - 1) / 2.0,
                0.0, -(image_height - 1) / 2.0, 0.0, (image_height - 1) / 2.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0;

    // 计算点云图
    Eigen::Matrix2Xf point_cloud = (mvp * mesh.colwise().homogeneous()).eval().colwise().hnormalized().topRows<2>();
    point_cloud.row(0) = ((point_cloud.row(0) / 2.0f).array() + 0.5f) * image.cols;
    point_cloud.row(1) = (0.5f - (point_cloud.row(1) / 2.0f).array()) * image.rows;
    
    Eigen::MatrixXf z;
    z.resize(1, mesh.cols());
    z = Eigen::MatrixXf::Ones(1, mesh.cols());
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mesh_homogeneous;
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> TranMesh_mvp;  // 定一个一个mesh_mvp转置之后的矩阵大小
    Eigen::Matrix<float, Eigen::Dynamic, 2> mesh_block;  // 定一个一个存储mesh 2D的矩阵
    Eigen::Matrix<float, 1, Eigen::Dynamic> chu;  // 定义mesh_mvp中要除的某一行存储空间。我们这里需要除mesh_mvp最后一行
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mesh_mvp;
    Eigen::Matrix<float, Eigen::Dynamic, 3> mesh_obj;
    Eigen::Matrix<uint16_t, Eigen::Dynamic, 3> mesh_F_obj;

    mesh_homogeneous.resize(mesh.rows()+1, mesh.cols());
    TranMesh_mvp.resize( mesh.cols(), mesh.rows()+1);
    mesh_block.resize(mesh.cols(), 2);
    mesh_obj.resize(mesh.cols(), 3);
    mesh_F_obj.resize(trig.cols(), 3);
    chu.resize(1, mesh.cols());
    mesh_mvp.resize(mesh.rows()+1, mesh.cols());

    mesh_homogeneous << mesh, z;  //实现按行concat
    std::cout<<"mesh_homogeneous:"<<mesh_homogeneous.rows()<<"  cols:"<<mesh_homogeneous.cols()<<std::endl;
    mesh_mvp = viewport * mvp * mesh_homogeneous;
    chu = mesh_mvp.row(3);  //将mesh_mvp最后一行数据赋值给chu；
    mesh_mvp = mesh_mvp.array().rowwise() / chu.array().row(0);  // 对mesh_mvp中的每一行都除以最后一行
    // mesh_mvp.row(3) = chu;  // 因为最后一行与自身除会等于1，而我们并不希望它被除，因此将它重新赋值回来。
    TranMesh_mvp = mesh_mvp.transpose();  // 对mesh_mvp进行转置；
    mesh_F_obj = trig.transpose(); // 对mesh中的F进行转置；
    mesh_obj = TranMesh_mvp.block<3520, 3>(0,0); // 为了保存投影后的mesh文件
    f.open(mesh_obj_path, std::ios::out);
    // 保存obj文件中的 V
    for(int i=0; i<mesh_obj.rows(); i++){
        f << "v "<<point_cloud.col(i)[0]<<" "<<point_cloud.col(i)[1]<<" "<<mesh_obj.row(i)[2]<<"\n";
    }
    // 保存obj文件中的 F
    for(int i=0; i<mesh_F_obj.rows(); i++){
        f << "f "<<mesh_F_obj.row(i)[0]+1<<" "<<mesh_F_obj.row(i)[1]+1<<" "<<mesh_F_obj.row(i)[2]+1<<"\n";
    }
    f.close();
    
    mesh_block = TranMesh_mvp.block<3520, 2>(0,0);  // 从（0，0）开始，找到一个大小为3520行, 2列的矩阵块；，从而得到我们需要的mesh 2D坐标；
    std::cout<<"mesh_block   row="<<mesh_block.rows()<<"cols="<<mesh_block.cols()<<std::endl;
    // std::cout<<"mesh_block"<<mesh_block<<std::endl;
    writeFileJson(point_cloud.transpose(), jsonfile, "mesh");
    return true;
}
void get_mesh_key(){
// 0. 获取版本信息
    std::cout << mtface_version() << std::endl;
    // 1. 创建静态图检测器， 此时它啥也做不了，因为还没有加载模型
    mtface_tracker_t *tracker = mtface_tracker_create(ThrowOnError());
    // 2. 加载需要的功能
    // 2.1 加载模型文件
    // 内部FD， 如果需要外部FD，则不加载 fd;

    mtface_tracker_append_model_file(tracker, mtface_type_fd, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_fd.bin", true, ThrowOnError());
    // 视频检测， 必须加载 FA！！！
    mtface_tracker_append_model_file(tracker, mtface_type_fa_medium, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_fa_medium.bin", true, ThrowOnError());
    mtface_tracker_append_model_file(tracker, mtface_type_dl3d, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_3d.bin", true, ThrowOnError());
    mtface_tracker_append_model_file(tracker, mtface_type_parsing, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_parsing.bin", true, ThrowOnError());
	// 3. 配置一些参数

    mtface_options_t *opts = mtface_tracker_get_options(tracker, nullptr, ThrowOnError());
    mtface_options_set(opts, mtface_option_minimal_face, 0.084);
    mtface_options_set(opts, mtface_option_score_threshold, 0.5);
    mtface_options_set(opts, mtface_option_enable_mesh_generation, true); // 生成3D点云

    mtface_tracker_set_options(tracker, opts, ThrowOnError());
    mtface_options_destroy(opts);

    opts = nullptr; // opts 不用试将其销毁
    cv::VideoCapture cap("/Users/wpx/Documents/video/celian.mp4");
	// width = int(cap::get(cv::CAP_PROP_FRAME_WIDTH));  
    // height = int(cap::get(cv::CAP_PROP_FRAME_HEIGHT));
    // cv::Mat cvbgr(width, height);
    cv::Mat texture = cv::imread("/Users/wpx/Documents/video/celian.jpg")/255;
	// cv::Mat cap = cv::imread("/Users/wpx/Documents/video/celian.jpg");
	int width, height;
    if(texture.data== nullptr)//nullptr是c++11新出现的空指针常量
    {
        std::cerr<<"图片文件不存在"<<std::endl;
    }else{
        std::cout<<"图片存在，very good！！！"<<std::endl;//你会发现图片就是一个矩阵
        std::cout<<"cols="<<texture.cols<<"  rows:="<<texture.rows<<"  cvbgr.step"<<texture.step<<std::endl;
    }
    mtface_image_t *image = nullptr;
    mtface_feature_t *feature = mtface_feature_create(ThrowOnError());
	bool flag = true;
    int num = 0;
    while (cap.isOpened()){
        cv::Mat frame;
        bool ret;
        cap.retrieve(frame);
        if (cap.grab()==false){
            break;
        }
        std::cout<<"frame cols="<<frame.cols<<"  rows:="<<frame.rows<<"  cvbgr.step"<<frame.step<<std::endl;
        image = mtface_image_adapt(image, frame.cols, frame.rows, mtface_pixel_format_bgr, 1, frame.data, frame.step,
                                   nullptr, 0, nullptr, 0, ThrowOnError());
        
        mtface_tracker_detect(tracker, image, feature, ThrowOnError());
        num++;
        if (num % 130 != 0){
            continue;
        }
        for (int i = 0; i < mtface_feature_get_face_size(feature); ++i){
            Eigen::Matrix4f mvp;
			// Eigen::Matrix<double, Eigen::Dynamic, 2> points;
			// mtface_feature_get_face3d_mesh(feature, i, points)
            mtface_feature_get_face3d_mvp(feature, i, mvp.data());
            size_t n = 0;
            size_t m = 0;
            size_t p = 0;
            const float *mesh = mtface_feature_get_face3d_mesh(feature, i, &n);  //得到网格点
            const uint16_t *trig = mtface_feature_get_face3d_triangle_list(feature, i, &m) ;  //得到三角网格点，就是你生成obj文件中的 f
            const float *key_point = mtface_feature_get_face_landmark(feature, i, &p) ;  // 得到的130个FA点

            Eigen::Map<const Eigen::Matrix3Xf> mesh_matrix(mesh, 3, n);
            // Eigen::Matrix3Xf mesh_matrix(mesh, 3, n);
            std::cout<<"n:"<<n<<" m="<<m<<" p="<<p<<" cols:\n"<<mesh_matrix.col(3)<<std::endl;
        }
    }
    mtface_tracker_destroy(tracker);
    mtface_feature_destroy(feature);
    mtface_options_destroy(opts);
    mtface_image_destroy(image);    
}
void testVideo3D(std::string image_path, std::string mesh_json, std::string keypoint_path, std::string mesh_obj_path)
{
    // 0. 获取版本信息
    std::cout << mtface_version() << std::endl;
    // 1. 创建静态图检测器， 此时它啥也做不了，因为还没有加载模型
    mtface_tracker_t *tracker = mtface_tracker_create(ThrowOnError());
    // 2. 加载需要的功能
    // 2.1 加载模型文件
    // 内部FD， 如果需要外部FD，则不加载 fd;

    mtface_tracker_append_model_file(tracker, mtface_type_fd, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_fd.bin", true, ThrowOnError());
    // 视频检测， 必须加载 FA！！！
    mtface_tracker_append_model_file(tracker, mtface_type_fa_medium, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_fa_medium.bin", true, ThrowOnError());
    mtface_tracker_append_model_file(tracker, mtface_type_dl3d, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_3d.bin", true, ThrowOnError());
    mtface_tracker_append_model_file(tracker, mtface_type_parsing, "/Users/wpx/Documents/arap_v2/3rdparty/libmtface/macos_shared/models/mtface_parsing.bin", true, ThrowOnError());
	// 3. 配置一些参数

    mtface_options_t *opts = mtface_tracker_get_options(tracker, nullptr, ThrowOnError());
    mtface_options_set(opts, mtface_option_minimal_face, 0.084);
    mtface_options_set(opts, mtface_option_score_threshold, 0.5);
    mtface_options_set(opts, mtface_option_enable_mesh_generation, true); // 生成3D点云

    mtface_tracker_set_options(tracker, opts, ThrowOnError());
    mtface_options_destroy(opts);

    opts = nullptr; // opts 不用试将其销毁
    // cv::VideoCapture cap("/Users/wpx/Documents/video/celian.mp4");
	// width = int(cap::get(cv::CAP_PROP_FRAME_WIDTH));  
    // height = int(cap::get(cv::CAP_PROP_FRAME_HEIGHT));
    // cv::Mat cvbgr(width, height);
    cv::Mat cvbgr = cv::imread(image_path);
	// cv::Mat cap = cv::imread("/Users/wpx/Documents/video/celian.jpg");
	int width, height;
    if(cvbgr.data== nullptr)//nullptr是c++11新出现的空指针常量
    {
        std::cerr<<"图片文件不存在"<<std::endl;
    }else
        std::cout<<"图片存在，very good！！！"<<std::endl;//你会发现图片就是一个矩阵
        std::cout<<"cols="<<cvbgr.cols<<"  rows:="<<cvbgr.rows<<"  cvbgr.step"<<cvbgr.step<<std::endl;

    mtface_image_t *image = nullptr;
    mtface_feature_t *feature = mtface_feature_create(ThrowOnError());
	bool flag = true;
    int countss = 0;
    while (flag)
    {
        if (cvbgr.empty())
        {
            break;
        }
        image = mtface_image_adapt(image, cvbgr.cols, cvbgr.rows, mtface_pixel_format_bgr, 1, cvbgr.data, cvbgr.step,
                                   nullptr, 0, nullptr, 0, ThrowOnError());
        // std::cout<<"image:"<<image<<std::endl;
        mtface_tracker_detect(tracker, image, feature, ThrowOnError());
        // show result
        int size = mtface_feature_get_face_size(feature);
        // std::cout<<"size="<<mtface_feature_get_face_size(feature)<<std::endl;
        for (int i = 0; i < mtface_feature_get_face_size(feature); ++i)
        {
            Eigen::Matrix4f mvp;
			// Eigen::Matrix<double, Eigen::Dynamic, 2> points;
			// mtface_feature_get_face3d_mesh(feature, i, points)
            mtface_feature_get_face3d_mvp(feature, i, mvp.data());
            size_t n = 0;
            size_t m = 0;
            size_t p = 0;
            const float *mesh = mtface_feature_get_face3d_mesh(feature, i, &n);  //得到网格点
            const uint16_t *trig = mtface_feature_get_face3d_triangle_list(feature, i, &m) ;  //得到三角网格点，就是你生成obj文件中的 f
            const float *key_point = mtface_feature_get_face_landmark(feature, i, &p) ;  // 得到的130个FA点
            // std::cout<<"trig:"<<trig<<std::endl;
            Eigen::Map<const Eigen::Matrix3Xf> mesh_matrix(mesh, 3, n);
            Eigen::Map<const Eigen::Matrix2Xf> key_point_matrix(key_point, 2, p);
            Eigen::Map<const Eigen::Matrix<uint16_t, 3, Eigen::Dynamic>> trig_matrix(trig, 3, m);
            std::cout<<"n:"<<n<<" m="<<m<<" p="<<p<<"\n cols:"<<mesh_matrix.col(3)<<std::endl;
            // std::cout<<"first mesh.cols:"<<mesh_matrix.cols()<<" mesh.rows="<<mesh_matrix.rows()<<std::endl;
            ThreeDimTo2Dim(cvbgr, mesh_matrix, mvp, trig_matrix, mesh_json, mesh_obj_path);
            std::cout<<"已经将将网格点保存到mesh_json.json"<<std::endl;
            writeFileJson(key_point_matrix.transpose(), keypoint_path, "keypoint");
            std::cout<<"已经将将FA点保存到keypoint_json.json"<<std::endl;
            // std::cout<<"mesh.cols:"<<mesh_matrix.cols()<<" mesh.rows="<<mesh_matrix.rows()<<" p="<<p<<" cols:\n"<<mesh_matrix.col(3)<<std::endl;
            Eigen::Matrix2Xf point_cloud = (mvp * mesh_matrix.colwise().homogeneous()).eval().colwise().hnormalized().topRows<2>();
            point_cloud.row(0) = ((point_cloud.row(0) / 2.0f).array() + 0.5f) * cvbgr.cols;
            point_cloud.row(1) = (0.5f - (point_cloud.row(1) / 2.0f).array()) * cvbgr.rows;
            
            // utility::plotLandmark(cvbgr, point_cloud.data(), point_cloud.cols());
            // for(int s=0;s<10; s++){
            //     std::cout<<"point_cloud"<<i<<"   col="<<point_cloud.col(s)<<std::endl;
            // }
            
        }
        // cv::imshow("video", cvbgr);
        // auto c = cv::waitKey(30);
        // if (c == 27)
        // {
        //     break;
        // }
        if (size!=0){
            flag = false;
        }else{
            countss++;
            // std::cout<<"countss:="<<countss<<std::endl;
            flag=true;
        }
        
    }
    mtface_tracker_destroy(tracker);
    mtface_feature_destroy(feature);
    mtface_options_destroy(opts);
    mtface_image_destroy(image);
    
    
}
// ---------------------------------------------------------------调用人脸库code----------------------------------------
int main(int argc, char *argv[])
{
	clock_t start,end;
    
    //------------------------读取mesh点和keypoint点的code-----------------------
    std::string image_path = argv[1];
    std::string mesh_path="/Users/wpx/Documents/arap_v2/jsons/mesh_json.json";
    std::string keypoint_path="/Users/wpx/Documents/arap_v2/jsons/keypoint_json.json";
    std::string mesh_obj_path = "/Users/wpx/Documents/arap_v2/jsons/mesh.obj";
    testVideo3D(image_path, mesh_path, keypoint_path, mesh_obj_path);
    //------------------------读取mesh点和keypoint点的code-----------------------

    //------------------------计算距离keypoing点最近的mesh点下标-----------------------
    std::string keyname = "keypoint";
    std::string meshname = "mesh";
    Eigen::MatrixX2f key2D;
    Eigen::MatrixX2f mesh2D;
    Eigen::MatrixX3f distan;
    key2D = readFileJson(keypoint_path, keyname);
    mesh2D = readFileJson(mesh_path, meshname);
    distan = computer_distance(key2D, mesh2D);   // 返回的是【keypoint， mesh_point】,都表示下标，比如对于74号keypoint，其对应的mesh_point为2318.
    std::cout<<"dis:"<<distan.row(74)<<std::endl;
    //------------------------计算距离keypoing点最近的mesh点下标-----------------------

    //------------------------进行鼻梁自动移动变换----------------------------------------
    start = clock(); 
    Mesh mesh;
    // std::string pathsss="/Users/wpx/Documents/arap_v2/data/celianMeshMvp.obj";
    if (!load_model(mesh_obj_path, mesh)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    LaplacianSystem system;
    

    Eigen::Matrix<int, 4, 2> key_mesh;
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> move_vec;
    move_vec.resize(4,1);
    move_vec << 920,  //对应71号点
                1276,  //对应72号点
                2412,  //对应73号点 
                2318; //对应74号点
                // 2318, 2335,2322; //对应74号点
                             
    int j = 0;
    for(int i=71; i<75; i++){  //只获取鼻梁71到74号点最近的mesh点
        key_mesh.row(j)[0] = int(distan.row(i)[0]);
        key_mesh.row(j)[1] = int(distan.row(i)[1]);
        std::cout<<"point="<<key_mesh.row(j)[0]<<"  mesh="<<key_mesh.row(j)[1]<<std::endl;
        j++;
    }
    std::cout<<"key_mesh_vet:"<<key_mesh<<std::endl;

    std::vector<FixedVertex> nose_fixed_vertices;
    for(int i=0;i<key_mesh.rows(); i++){
        size_t curgrp = 0;
        int fixvec = key_mesh.row(i)[1]; 
        nose_fixed_vertices.push_back({key_mesh.row(i)[1], float(move_vec.row(i)[0])}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp
        
    }
    // std::sort(nose_fixed_vertices.begin(), nose_fixed_vertices.end(), compare_by_index);
    // vertic_move(mesh, nose_fixed_vertices, 1); 
    end = clock();  
    std::cout<<"time = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
    // std::cout << mesh.V.rows() << ", " << elapsed.count() / iterations << std::endl;
   
    //------------------------进行鼻梁自动移动变换----------------------------------------

    //------------------------进行人脸轮廓（FA点在mesh外部）自动移动变换----------------------------------------
    
    Mesh meshs;
    // std::string pathsss="/Users/wpx/Documents/arap_v2/data/celianMeshMvp.obj";
    if (!load_model("/Users/wpx/Documents/arap_v2/build/examples/mesh_deformation.obj", meshs)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    int fj = 0;
    Eigen::Matrix<float, Eigen::Dynamic, 3> left_FM_key_mesh;  // 左边脸优化
    Eigen::Matrix<float, Eigen::Dynamic, 3> right_FM_key_mesh; // 右边脸优化
    left_FM_key_mesh.resize(17, 3);
    right_FM_key_mesh.resize(16, 3);
    for(int i=0; i<17; i++){  //只获取鼻梁71到74号点最近的mesh点
        // if (int(distan.row(i)[2]) > 25){
            left_FM_key_mesh.row(fj)[0] = distan.row(i)[0];  // FA点
            left_FM_key_mesh.row(fj)[1] = distan.row(i)[1];  // 距离FA最近的Mesh点
            left_FM_key_mesh.row(fj)[2] = distan.row(i)[2];  // 距离FA最近Mesh点之间的距离
            fj++;
        // }           
    }
    int fjs = 0;
    for(int i=17; i<33; i++){  //只获取鼻梁71到74号点最近的mesh点
        // if (int(distan.row(i)[2]) > 25){
            right_FM_key_mesh.row(fjs)[0] = distan.row(i)[0];  // FA点
            right_FM_key_mesh.row(fjs)[1] = distan.row(i)[1];  // 距离FA最近的Mesh点
            right_FM_key_mesh.row(fjs)[2] = distan.row(i)[2];  // 距离FA最近Mesh点之间的距离
            fjs++;
        // }           
    }

    // Eigen::Matrix<float, couFM, 1> fixvec;
    for (int j=0; j<2; j++){
        std::vector<FixedVertex> fixed_vertices;
        if (j==0){
            for(int i=0; i<left_FM_key_mesh.rows(); i++){
                 if (i % 4==0){
                     // std::cout<<"left:  "<<left_FM_key_mesh.row(i)[1]<<std::endl;
                    fixed_vertices.push_back({int(left_FM_key_mesh.row(i)[1]), left_FM_key_mesh.row(i)[2]}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp   
                 }
                // // std::cout<<"left:  "<<left_FM_key_mesh.row(i)[1]<<std::endl;
                // fixed_vertices.push_back({int(left_FM_key_mesh.row(i)[1]), left_FM_key_mesh.row(i)[2]}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp   
            }
        }else{
            for(int i=0; i<right_FM_key_mesh.rows(); i++){
                // std::cout<<"right:  "<<right_FM_key_mesh.row(i)[1]<<std::endl;
                fixed_vertices.push_back({int(right_FM_key_mesh.row(i)[1]), right_FM_key_mesh.row(i)[2]}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp   
            }
        }
        // std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
        // std::reverse(fixed_vertices.begin(), fixed_vertices.end());
        Face_Contour_Restoration(mesh, fixed_vertices, 1, j); 
    }
    
    
    //------------------------进行人脸轮廓（FA点在mesh外部）自动移动变换----------------------------------------
    return 0;

}
