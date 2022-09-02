
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
#include <tuple>
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
	igl::writeOBJ("1_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	std::cout<<mesh.V.rows()<<std::endl;
}

void vertic_move(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int fixvec, int iterations) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }

    for (const auto& vertex : fixed_vertices) {

        if (vertex.index!=fixvec){
            // std::cout<<"vertex.index:"<<vertex.index<<"  fixed_vertices[0].index:"<<system.mesh->V.row(fixed_vertices[0].index)<<std::endl;
            system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec);
        }		
	}
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save nose mesh: mesh_deformation.obj"<<std::endl;
    igl::writeOBJ("1_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
}
void vertic_move_v1(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int fixvec, Eigen::MatrixX2f FN_key, int iterations) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
    
    for (const auto& vertex : fixed_vertices) {

        float flag;
        for(int i=0; i<FN_key.rows(); i++){
            if(FN_key.row(i)[0]==float(vertex.index)){
                flag = FN_key.row(i)[1];
            }
        }
        
        if ((vertex.index!=fixvec) && flag == 0 || flag == 1 || flag == 2 || flag == 3){
            // std::cout<<"flag: "<<flag<<std::endl;
            // std::cout<<"vertex.index:"<<vertex.index<<"  fixed_vertices[0].index:"<<system.mesh->V.row(fixed_vertices[0].index)<<std::endl;
            system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec);
        }		
	}
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save nose mesh: 1_mesh_deformation.obj"<<std::endl;
    igl::writeOBJ("1_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
}

void vertic_move_v2(Mesh mesh, std::vector<FixedVertex> fixed_vertices, Eigen::Matrix4Xf fixvec, Eigen::MatrixX2f FN_key, int iterations) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
    for (const auto& vertex : fixed_vertices) {
        float flag;
        for(int i=0; i<FN_key.rows(); i++){
            if(FN_key.row(i)[0]==float(vertex.index)){
                flag = FN_key.row(i)[1];
            }
        }
        
        if ((vertex.index!=fixvec.row(0)[0]) && flag == 0){
            std::cout<<"1flag: "<<flag<<std::endl;
            system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec.row(0)[0]);
        }else if((vertex.index!=fixvec.row(1)[0]) && flag == 1 ){
            std::cout<<"2flag: "<<flag<<std::endl;
            system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec.row(1)[0]);
        }else if((vertex.index!=fixvec.row(2)[0]) &&  flag == 2){
            std::cout<<"3flag: "<<flag<<std::endl;
            system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec.row(2)[0]);
        }else if((vertex.index!=fixvec.row(3)[0]) && flag == 3){
            std::cout<<"4flag: "<<flag<<std::endl;
            system.mesh->V.row(vertex.index) = system.mesh->V.row(fixvec.row(3)[0]);
        }
    }
    
    
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save nose mesh: 1_mesh_deformation.obj"<<std::endl;
    igl::writeOBJ("1_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
}
void Face_Contour_Restoration(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations, int fu) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }

    for (const auto& vertex : fixed_vertices) {
        float time = vertex.group;
        if (fu==0 && time >0){
            // float futime = time;  //小分辨率图像 400X400左右
            float futime = time + 0.012;
            std::cout<<"1fu:"<<fu<<" time= "<<vertex.group<<" index:"<<vertex.index<<std::endl;
            time = time + 1;
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[0] * futime;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * (futime - futime * 0.85);	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime;
        }else if(time > 0){
            std::cout<<"2fu:"<<fu<<" time="<<time<<std::endl;
            if (time > 0.2){
                time = time * 0.5 + 1;
            }else{
                time = time + 1;
            }
            
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
    igl::writeOBJ("1_face_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	std::cout<<mesh.V.rows()<<std::endl;  
}

void Face_Contour_Restoration_v1(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations, int fu, Eigen::Matrix<float, Eigen::Dynamic, 2> FM_key) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
    int count = 0;
    for (const auto& vertex : fixed_vertices) {
        float time = vertex.group;
        float flag;
        for(int i=0; i<FM_key.rows(); i++){
            if(FM_key.row(i)[0]==float(vertex.index)){
                flag = FM_key.row(i)[1];
            }
        }
        if (flag==0 && time > 0){
            // float futime = time;  //小分辨率图像 400X400左右
            float futime = time + 0.012;
            // std::cout<<"1 fu:"<<fu<<" time= "<<vertex.group<<" index:"<<vertex.index<<std::endl;
            time = time + 1;
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[0] * futime;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * (futime - futime * 0.85);	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime; 
        }else if(flag==1 && time > 0){
            // std::cout<<"2 fu:"<<fu<<" time="<<time<<std::endl;
            if (time > 0.2){
                time = time * 0.5 + 1;
            }else{
                time = time + 1;
            }
            
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] * time;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] * time;	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] * time;
        }
        count++;
    }
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save mesh face: face_mesh_deformation.obj"<<std::endl;
    igl::writeOBJ("1_face_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
}

void Face_Contour_Restoration_v2(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations, int fu, Eigen::Matrix<float, Eigen::Dynamic, 2> FM_key, float image_ration) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
    int count = 0;
    for (const auto& vertex : fixed_vertices) {

        // std::cout<<"vertex.index:"<<vertex.index<<"  fixed_vertices[0].index:"<<system.mesh->V.row(fixed_vertices[0].index)<<std::endl;
        // double time = (1 + (double(vertex.group)/(double)550));
        // double time = (1 + (double(vertex.group)/double(vertex.group + vertex.group * 0.02)));
        
        float time = vertex.group;
        float flag;
        for(int i=0; i<FM_key.rows(); i++){
            if(FM_key.row(i)[0]==float(vertex.index)){
                flag = FM_key.row(i)[1];
            }
        }
        float ration = image_ration * 0.25;
        if (flag==0 && time > 0){
            // float futime = time;  //小分辨率图像 400X400左右
            float futime = time + ration ;
            // std::cout<<"1 fu:"<<fu<<" time= "<<vertex.group<<" index:"<<vertex.index<<std::endl;
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[0] * futime;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * (futime - futime * 0.9);	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime; 
        }else if(flag==1 && time > 0){
            std::cout<<" time= "<<time<<" ration: "<<ration<<std::endl;
            if (time > 0.2){
                time = (time + ration) *  0.50 + 1;
            }else{
                time = (time + ration) *  0.50 + 1;
            }
            
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] * time;
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] * time;	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] * time;
        }
        count++;
    }
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save mesh face: face_mesh_deformation.obj"<<std::endl;
    igl::writeOBJ("1_face_mesh_deformation.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
}
void ForeFeadRestoration(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations, int fu, Eigen::Matrix<float, Eigen::Dynamic, 2> FM_key) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
    int count = 0;
    for (const auto& vertex : fixed_vertices) {

        // std::cout<<"vertex.index:"<<vertex.index<<"  fixed_vertices[0].index:"<<system.mesh->V.row(fixed_vertices[0].index)<<std::endl;
        // double time = (1 + (double(vertex.group)/(double)550));
        // double time = (1 + (double(vertex.group)/double(vertex.group + vertex.group * 0.02)));
        
        float time = vertex.group;
        float flag;
        for(int i=0; i<FM_key.rows(); i++){
            if(FM_key.row(i)[0]==float(vertex.index)){
                flag = FM_key.row(i)[1];
            }
        }
        if (flag==2 && time > 0){ // 移动左额头点
            // float futime = time;  //小分辨率图像 400X400左右
            float futime = time + 0.02;
            // std::cout<<"1 fu:"<<fu<<" time= "<<vertex.group<<" index:"<<vertex.index<<std::endl;
            time = time + 1;
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[1] * (futime - futime * 0.25);
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * (futime);	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime; 
        
        }else if(flag==3 && time > 0){  //移动右额头点
            float futime = time + 0.042;
            // std::cout<<"1 fu:"<<fu<<" time= "<<vertex.group<<" index:"<<vertex.index<<std::endl;
            time = time + 1;
            system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] + system.mesh->V.row(vertex.index)[1] * (futime - futime * 0.5);
            system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * (futime);	
            system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime; 
        
        }
        count++;
    }
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save mesh face: ForeHeadRestoration.obj"<<std::endl;
    igl::writeOBJ("1_ForeHeadRestoration.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
}
void ForeFeadRestoration_v1(Mesh mesh, std::vector<FixedVertex> fixed_vertices, int iterations, int fu, Eigen::Matrix<float, Eigen::Dynamic, 2> FM_key, Eigen::Matrix2Xf image_inf, double means) {
    using namespace std::chrono;
    LaplacianSystem system;
	
    system_init(system, &mesh, 0.05);
    if (!system_bind1(system, fixed_vertices)) {
		return;
    }
    for (const auto& vertex : fixed_vertices) {
        float time = vertex.group;
        float flag;
        for(int i=0; i<FM_key.rows(); i++){
            if(FM_key.row(i)[0]==float(vertex.index)){
                flag = FM_key.row(i)[1];
            }
        }

        float ration;
        float image_ration=image_inf.row(0)[0], image_width = image_inf.row(1)[0];
        ration = ((means/image_width)/image_ration)/10;
        // std::cout<<"ration "<<ration<<std::endl;
        if (ration > 0.04){
            ration = ration * 0.7;
        }
        if (flag<13){ // 移动左额头点
            std::cout<<"ration "<<ration<<std::endl;
            
            if(time > 0 && ration>0.005){
                
                float futime = time + ration * (ration * 100);
                std::cout<<"1futime: "<<futime<<"  flag: "<<flag<<std::endl;
                if (flag >5){  // 左额头：5-13号点，向上移动权重大一点
                    system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[1] * (futime * 0.9);
                    system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * futime;	
                    system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime; 
                }else{ // 左额头：0-5号点，向上移动权重小一点
                    system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] - system.mesh->V.row(vertex.index)[1] * (futime * 0.9);
                    system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] + system.mesh->V.row(vertex.index)[1] * futime * 0.05;	
                    system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime; 
                }
            } 
        }else if(flag>=13){  //移动右额头点
            if(time > 0 && ration>0.005){
                float futime = time + ration * (ration * 100);
                std::cout<<"2futime: "<<futime<<"  flag:"<<flag<<std::endl;
                if (flag < 19){
                    system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] + system.mesh->V.row(vertex.index)[1] * (futime * 0.9);
                    system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] - system.mesh->V.row(vertex.index)[1] * futime;	
                    system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime;    
                }else{
                    system.mesh->V.row(vertex.index)[0] = system.mesh->V.row(vertex.index)[0] + system.mesh->V.row(vertex.index)[1] * (futime * 0.9);
                    system.mesh->V.row(vertex.index)[1] = system.mesh->V.row(vertex.index)[1] + system.mesh->V.row(vertex.index)[1] * futime * 0.02;
                    system.mesh->V.row(vertex.index)[2] = system.mesh->V.row(vertex.index)[2] - system.mesh->V.row(vertex.index)[2] * futime;    
                }
            }
         }
    }
    auto t0 = high_resolution_clock::now();
    system_solve(system, iterations);
    auto t1 = high_resolution_clock::now();

    duration<double> elapsed(t1 - t0);
    std::cout<<"save mesh face: ForeHeadRestoration.obj"<<std::endl;
    igl::writeOBJ("1_ForeHeadRestoration.obj",system.mesh->V,system.mesh->F);
	// std::cout<<mesh.V.rows()<<std::endl;  
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

void norm_distance(double mids, Eigen::MatrixX3f &di, int start, int end, int image_width){
    // Z-score归一化方法
    double means;
    double sigma = 0;
    int key_point_num = end - start;
    means = mids / double(key_point_num);
    std::cout<<"means: "<<means<<"\n";
    for (int i=start; i < end; i++){
        sigma += pow((di.row(i)[2] - means), 2);
    }
    sigma = sigma / double(key_point_num);
    for(int i=start; i < end; i++){
        /*
            从15个案例【半脸遮挡，大范围遮挡，小鼻子，中年-男，龅牙，中胡，
            方脸，眼镜，大鼻子，模糊，青年-男人脸边缘， 外头， 寸头， 帽子】的均值中
            得出的经验，均值低于10，mesh点都是越过FA点，不需要往外拉
        */
        if (image_width >650 && image_width<800 && means > 8){  // 对于侧脸情况，发现距离均值低于8，因为误差很小，可以考虑不用移动，否则会出现拉伸过度的情况发生。
            di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
        }else if(image_width < 650  && means > 5){
            di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
        }else{
            if (image_width<2400 && means>8){      // default: 10.5
                di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
            }else if ((image_width>=2400 && means>11)){
                di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
            }else{
                di.row(i)[2] = -1;
            }
        }    
    }
}
double norm_distance_forehead(double mids, Eigen::MatrixX3f &di, int start, int end, int image_width){
    // Z-score归一化方法
    double means;
    double sigma = 0;
    int key_point_num = end - start;
    means = mids / double(key_point_num);
    std::cout<<"means: "<<means<<"\n";
    for (int i=start; i < end; i++){
        sigma += pow((di.row(i)[2] - means), 2);
    }
    sigma = sigma / double(key_point_num);
    for(int i=start; i < end; i++){
        /*
            从15个案例【半脸遮挡，大范围遮挡，小鼻子，中年-男，龅牙，中胡，
            方脸，眼镜，大鼻子，模糊，青年-男人脸边缘， 外头， 寸头， 帽子】的均值中
            得出的经验，均值低于10，mesh点都是越过FA点，不需要往外拉
        */
        if (image_width >650 && image_width<800 && means > 8){  // 对于侧脸情况，发现距离均值低于8，因为误差很小，可以考虑不用移动，否则会出现拉伸过度的情况发生。
            di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
        }else if(image_width < 650 && means > 8){
            di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
        }else{
            if (means>10.5){      // default: 10.5
                di.row(i)[2] = ((di.row(i)[2] - means) / sigma) * 0.1;
            }else{
                di.row(i)[2] = -1;
            }
        }    
    }
    return means;
}
Eigen::MatrixX3i computer_distance(Eigen::MatrixX2f &key_point, Eigen::MatrixX2f &mesh_vet){
    int dis;
    Eigen::Matrix<int, Eigen::Dynamic, 3> di;
    Eigen::Matrix<float, 1,2> mesh;
    Eigen::Matrix<float, 1,2> key;
    int key_count = 0;
    Eigen::MatrixX2f::Index maxRow, maxCol;
    maxRow = mesh_vet.rows();
    maxCol = 0;
    di.resize(key_point.rows(), 3);
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
                // std::cout<<"di:"<<min<<std::endl;
                key_count++;
                break;
            }
        }   
    };
    return di;
};

Eigen::MatrixX3f computer_distance_v1(Eigen::MatrixX2f &key_point, Eigen::MatrixX2f &mesh_vet,  int image_width){
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
    // std::cout<<"key_point:"<<key_point.rows()<<std::endl;
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
    // std::cout<<"before: \n"<<di<<std::endl;
    norm_distance(mids, di, 0, 33, image_width);
    // std::cout<<"after:  \n"<<di<<std::endl;
    return di;
};


Eigen::MatrixX3f computer_distance_v2(Eigen::MatrixX2f &key_point, Eigen::MatrixX2f &mesh_vet, int image_width){
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
    double forehead_mid = 0;
    double mouse_mid = 0;
    // std::cout<<"key_point:"<<key_point.rows()<<std::endl;
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
                // 首先将锚点（眉毛+眼睛+鼻梁）存入到矩阵中: 总共40个点。用于归一化
                if (key_count < 40){
                    // std::cout<<key_count<<": "<<min<<std::endl;
                    mids += min;
                }
                // 这个判断语句是为了保存嘴巴点距离的总和，用于归一化
                if(key_count >= 40 && key_count < 60){
                    mouse_mid += min;
                }
                // 这个判断语句是为了保存额头点距离的总和，用于归一化
                if (key_count>=60){
                    forehead_mid += min;
                }
                key_count++;
                break;
            }
        }   
    };
    // std::cout<<"dis before: \n"<<di<<std::endl;
    /*
        试着去掉眉毛，眼睛，嘴巴处的归一化，结果就会出现形变成一条直线的情况。因此我保留了下来。
    */
    double means1, means2, means3;
    means1 = norm_distance_forehead(mids, di, 0, 40, image_width);   // 对眉毛和眼睛距离进行归一化
    means2 = norm_distance_forehead(mouse_mid, di, 40, 60, image_width);   // 对嘴巴距离进行归一化
    means3 = norm_distance_forehead(forehead_mid, di, 60, 87, image_width);  // 对额头点距离进行归一化
    // std::cout<<"dis after : \n"<<di<<std::endl;
    // std::cout<<"sigma: "<<sigma<<" means: "<<mids / double(33)<<std::endl;
    return di;
};

std::tuple<Eigen::MatrixX3f, double> computer_distance_v3(Eigen::MatrixX2f &key_point, Eigen::MatrixX2f &mesh_vet, int image_width){
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
    double forehead_mid = 0;
    double mouse_mid = 0;
    // std::cout<<"key_point:"<<key_point.rows()<<std::endl;
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
                // 首先将锚点（眉毛+眼睛+鼻梁）存入到矩阵中: 总共40个点。用于归一化
                if (key_count < 40){
                    // std::cout<<key_count<<": "<<min<<std::endl;
                    mids += min;
                }
                // 这个判断语句是为了保存嘴巴点距离的总和，用于归一化
                if(key_count >= 40 && key_count < 60){
                    mouse_mid += min;
                }
                // 这个判断语句是为了保存额头点距离的总和，用于归一化
                if (key_count>=60){
                    forehead_mid += min;
                }
                key_count++;
                break;
            }
        }   
    };
    // std::cout<<"dis before: \n"<<di<<std::endl;
    /*
        试着去掉眉毛，眼睛，嘴巴处的归一化，结果就会出现形变成一条直线的情况。因此我保留了下来。
    */
    std::tuple<Eigen::MatrixX3f, double> result;
    double means1, means2, means3;
    means1 = norm_distance_forehead(mids, di, 0, 40, image_width);   // 对眉毛和眼睛距离进行归一化
    means2 = norm_distance_forehead(mouse_mid, di, 40, 60, image_width);   // 对嘴巴距离进行归一化
    means3 = norm_distance_forehead(forehead_mid, di, 60, 87, image_width);  // 对额头点距离进行归一化
    // std::cout<<"dis after : \n"<<di<<std::endl;
    // std::cout<<"sigma: "<<sigma<<" means: "<<mids / double(33)<<std::endl;
    result = std::make_tuple(di, means3);
    return result;
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

Eigen::MatrixX2f forehead_readFileJson(std::string json_path, std::string filename, std::string landmarks)
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
        
        const Json::Value partner = root["person"];
        const Json::Value items = partner[0][filename];  // 找到额头点对应的2D点
        const Json::Value freepoint = partner[0][landmarks];  // 找到锚点对应的2D点，这里我们需要眼睛，眉毛哪一块的点，保证额头点在移动时，这区域的点不移动
        keypoint.resize((items.size() + 80 + 40)/2, 2);  // 我们只选取眉头和眼睛，鼻梁处的点，从33到73. 嘴巴：86到106
        Eigen::Vector2f vec;
        // std::cout<<"person partner:"<<partner.size()<<std::endl;
        int cou = 0;
        int vec_k=0;
        // 首先将锚点（眉毛+眼睛+鼻梁）存入到矩阵中: 总共40个点
        for (unsigned int j=64;j<144; j++){
            
            vec(vec_k++) = freepoint[j].asFloat();
            if ((j+1) % 2==0){
                // std::cout<<j<<"  "<<vec(0)<<" "<<vec(1)<<"\n";
                for (int k =0; k<2; k++){
                    keypoint(cou,k) = vec(k);
                }
                cou++;
                vec_k = 0;
            }
        }
        // 首先将锚点（嘴巴）存入到矩阵中  ：总共：20个点
        for (unsigned int j=170;j<210; j++){
            
            vec(vec_k++) = freepoint[j].asFloat();
            if ((j+1) % 2==0){
                // std::cout<<j<<"  "<<vec(0)<<" "<<vec(1)<<"\n";
                for (int k =0; k<2; k++){
                    keypoint(cou,k) = vec(k);
                }
                cou++;
                vec_k = 0;
            }
        }
        // 之后将要移动的额头点存入到矩阵中 ，总共27个点
        for (unsigned int j=0;j<items.size(); j++){
            
            vec(vec_k++) = items[j].asFloat();
            if ((j+1) % 2==0){
                for (int k =0; k<2; k++){
                    keypoint(cou,k) = vec(k);
                }
                cou++;
                vec_k = 0;
            }
        }

        
    }

    in.close();
    // std::cout<<keypoint<<std::endl;
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
    //---------------------------------------传统计算方法（结果有问题）-------------------------------
    // Eigen::MatrixXf z;
    // z.resize(1, mesh.cols());
    // z = Eigen::MatrixXf::Ones(1, mesh.cols());
    // Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mesh_homogeneous;
    // Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> TranMesh_mvp;  // 定一个一个mesh_mvp转置之后的矩阵大小
    // Eigen::Matrix<float, Eigen::Dynamic, 2> mesh_block;  // 定一个一个存储mesh 2D的矩阵
    // Eigen::Matrix<float, 1, Eigen::Dynamic> chu;  // 定义mesh_mvp中要除的某一行存储空间。我们这里需要除mesh_mvp最后一行
    // Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mesh_mvp;
    // Eigen::Matrix<float, Eigen::Dynamic, 3> mesh_obj;
    // Eigen::Matrix<uint16_t, Eigen::Dynamic, 3> mesh_F_obj;

    // mesh_homogeneous.resize(mesh.rows()+1, mesh.cols());
    // TranMesh_mvp.resize( mesh.cols(), mesh.rows()+1);
    // mesh_block.resize(mesh.cols(), 2);
    // mesh_obj.resize(mesh.cols(), 3);
    // mesh_F_obj.resize(trig.cols(), 3);
    // chu.resize(1, mesh.cols());
    // mesh_mvp.resize(mesh.rows()+1, mesh.cols());

    // mesh_homogeneous << mesh, z;  //实现按行concat
    // // std::cout<<"mesh_homogeneous:"<<mesh_homogeneous.rows()<<"  cols:"<<mesh_homogeneous.cols()<<std::endl;
    // mesh_mvp = viewport * mvp * mesh_homogeneous;
    // chu = mesh_mvp.row(3);  //将mesh_mvp最后一行数据赋值给chu；
    // mesh_mvp = mesh_mvp.array().rowwise() / chu.array().row(0);  // 对mesh_mvp中的每一行都除以最后一行
    // // mesh_mvp.row(3) = chu;  // 因为最后一行与自身除会等于1，而我们并不希望它被除，因此将它重新赋值回来。
    // TranMesh_mvp = mesh_mvp.transpose();  // 对mesh_mvp进行转置；
    // mesh_F_obj = trig.transpose(); // 对mesh中的F进行转置；
    // mesh_obj = TranMesh_mvp.block<3520, 3>(0,0); // 为了保存投影后的mesh文件
    // f.open(mesh_obj_path, std::ios::out);
    // // 保存obj文件中的 V
    // for(int i=0; i<mesh_obj.rows(); i++){
    //     f << "v "<<point_cloud.col(i)[0]<<" "<<point_cloud.col(i)[1]<<" "<<mesh_obj.row(i)[2]<<"\n";
    // }
    // // 保存obj文件中的 F
    // for(int i=0; i<mesh_F_obj.rows(); i++){
    //     f << "f "<<mesh_F_obj.row(i)[0]+1<<" "<<mesh_F_obj.row(i)[1]+1<<" "<<mesh_F_obj.row(i)[2]+1<<"\n";
    // }
    // f.close();
    
    // mesh_block = TranMesh_mvp.block<3520, 2>(0,0);  // 从（0，0）开始，找到一个大小为3520行, 2列的矩阵块；，从而得到我们需要的mesh 2D坐标；
    // std::cout<<"mesh_block   row="<<mesh_block.rows()<<"cols="<<mesh_block.cols()<<std::endl;
    // std::cout<<"mesh_block"<<mesh_block<<std::endl;
    //---------------------------------------传统计算方法（结果有问题）-------------------------------
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
int testVideo3D(std::string image_path, std::string mesh_json, std::string keypoint_path, std::string mesh_obj_path)
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
    float face_ratio;
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
            auto rect = mtface_feature_get_face_box(feature, i);
            // cv::Rect box(rect.x, rect.y, rect.width, rect.height);
            face_ratio = float((rect.width * rect.height)) / float((cvbgr.cols * cvbgr.rows));// 计算人脸占据整张图片面积
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
            // std::cout<<"n:"<<n<<" m="<<m<<" p="<<p<<"\n cols:"<<mesh_matrix.col(3)<<std::endl;
            // std::cout<<"first mesh.cols:"<<mesh_matrix.cols()<<" mesh.rows="<<mesh_matrix.rows()<<std::endl;
            ThreeDimTo2Dim(cvbgr, mesh_matrix, mvp, trig_matrix, mesh_json, mesh_obj_path);
            std::cout<<"已经将网格点保存到mesh_json.json"<<std::endl;
            writeFileJson(key_point_matrix.transpose(), keypoint_path, "keypoint");
            std::cout<<"已经将FA点保存到keypoint_json.json"<<std::endl;
            // std::cout<<"mesh.cols:"<<mesh_matrix.cols()<<" mesh.rows="<<mesh_matrix.rows()<<" p="<<p<<" cols:\n"<<mesh_matrix.col(3)<<std::endl;
            Eigen::Matrix2Xf point_cloud = (mvp * mesh_matrix.colwise().homogeneous()).eval().colwise().hnormalized().topRows<2>();
            point_cloud.row(0) = ((point_cloud.row(0) / 2.0f).array() + 0.5f) * cvbgr.cols;
            point_cloud.row(1) = (0.5f - (point_cloud.row(1) / 2.0f).array()) * cvbgr.rows;
            
        }
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
    int min_width;
    if (cvbgr.rows > cvbgr.cols){
        min_width = cvbgr.cols;
    }else{
        min_width = cvbgr.rows;
    }
    
    // return cvbgr.cols;
    // std::cout<<"face_ratio: "<<face_ratio<<std::endl;
    return min_width;
}

Eigen::Matrix2Xf testVideo3D_v1(std::string image_path, std::string mesh_json, std::string keypoint_path, std::string mesh_obj_path)
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
    float face_ratio;
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
            auto rect = mtface_feature_get_face_box(feature, i);
            // cv::Rect box(rect.x, rect.y, rect.width, rect.height);
            face_ratio = float((rect.width * rect.height)) / float((cvbgr.cols * cvbgr.rows));// 计算人脸占据整张图片面积
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
            // std::cout<<"n:"<<n<<" m="<<m<<" p="<<p<<"\n cols:"<<mesh_matrix.col(3)<<std::endl;
            // std::cout<<"first mesh.cols:"<<mesh_matrix.cols()<<" mesh.rows="<<mesh_matrix.rows()<<std::endl;
            ThreeDimTo2Dim(cvbgr, mesh_matrix, mvp, trig_matrix, mesh_json, mesh_obj_path);
            std::cout<<"已经将网格点保存到mesh_json.json"<<std::endl;
            writeFileJson(key_point_matrix.transpose(), keypoint_path, "keypoint");
            std::cout<<"已经将FA点保存到keypoint_json.json"<<std::endl;
            // std::cout<<"mesh.cols:"<<mesh_matrix.cols()<<" mesh.rows="<<mesh_matrix.rows()<<" p="<<p<<" cols:\n"<<mesh_matrix.col(3)<<std::endl;
            Eigen::Matrix2Xf point_cloud = (mvp * mesh_matrix.colwise().homogeneous()).eval().colwise().hnormalized().topRows<2>();
            point_cloud.row(0) = ((point_cloud.row(0) / 2.0f).array() + 0.5f) * cvbgr.cols;
            point_cloud.row(1) = (0.5f - (point_cloud.row(1) / 2.0f).array()) * cvbgr.rows;
        }
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
    int min_width;
    if (cvbgr.rows > cvbgr.cols){
        min_width = cvbgr.cols;
    }else{
        min_width = cvbgr.rows;
    }
    
    // return cvbgr.cols;
    std::cout<<"face_ratio: "<<face_ratio<<std::endl;
    Eigen::Matrix2Xf result;
    result.resize(2,1);
    result.row(0)[0] = face_ratio;
    result.row(1)[0] = min_width;
    return result;
}
// ---------------------------------------------------------------调用人脸库code----------------------------------------

// 实现鼻子修复
namespace fs = std::filesystem;  // 这个主要是为了获取当前目录的路径

int nose_main(int argc, char *argv[])
{
	clock_t start,end;
    start = clock(); 
    int image_width;
    std::cout<<"currend dir: "<<fs::current_path()<<std::endl;
    //------------------------读取mesh点和keypoint点的code-----------------------
    std::string image_path = argv[1];   // 输入的优化图片
    std::string root = std::string(fs::current_path());
    std::string mesh_path = root + "/1_mesh_json.json";
    std::string keypoint_path= root + "/1_keypoint_json.json";
    std::string mesh_obj_path =  root + "/1_mesh.obj";
    std::string nose_obj_path = root + "/1_mesh_deformation.obj";
    image_width = testVideo3D(image_path, mesh_path, keypoint_path, mesh_obj_path);  // image_width 这里表示从width 和height选择最小的值。
    //------------------------读取mesh点和keypoint点的code-----------------------

    //------------------------计算距离keypoing点最近的mesh点下标-----------------------
    std::string keyname = "keypoint";
    std::string meshname = "mesh";
    Eigen::MatrixX2f FH_key2D;  // FH 表示 ForeHead
    Eigen::MatrixX2f key2D;
    Eigen::MatrixX2f mesh2D;

    Eigen::MatrixX3f distan;
    key2D = readFileJson(keypoint_path, keyname);  // 需要mtface生成的关键点数据
    mesh2D = readFileJson(mesh_path, meshname);    // 读取生成的人脸mesh数据
    
    distan = computer_distance_v1(key2D, mesh2D, image_width);   // 返回的是130个FA点最近mesh距离，其中0-33号点进行了归一化，用于计算权重。鼻梁点只需要找到最近的mesh点，不需要归一化。 
    // std::cout<<"distan   :"<<distan<<std::endl;
    // std::cout<<"FH_distan:"<<FH_distan<<"\n ----------------\n";
    //------------------------计算距离keypoing点最近的mesh点下标-----------------------

    //------------------------进行鼻梁自动移动变换----------------------------------------
    std::cout<<"开始鼻子修复"<<std::endl;
    Mesh mesh;
   
    if (!load_model(mesh_obj_path, mesh)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    LaplacianSystem system;
    

    Eigen::Matrix<int, 4, 2> key_mesh;
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> move_vec;
    move_vec.resize(4,3);
    move_vec <<208, 931, 1803,  //对应71号点
                828, 923, 1276,  //对应72号点
                2412,2478,2276,  //对应73号点 
                2318, 2322,2318; //对应74号点
                // 2318, 2335,2322; //对应74号点
                             
    int j = 0;
    int FN_cou = 0;
    Eigen::Matrix<float, Eigen::Dynamic, 2> FN_key; 
    FN_key.resize(12+19+20, 2);  //4表示鼻梁点，18表示眼睛点，20表示嘴巴点

    for(int i=71; i<75; i++){  //只获取鼻梁71到74号点最近的mesh点
        key_mesh.row(j)[0] = int(distan.row(i)[0]);
        key_mesh.row(j)[1] = int(distan.row(i)[1]);
        j++;
    }
    // 用于记录鼻子点
    int nose_c = 0;
    for(int i=0; i<move_vec.rows(); i++){
        for (int j=0; j <move_vec.cols(); j++){
            FN_key.row(FN_cou)[0] = move_vec.row(i)[j];
            FN_key.row(FN_cou)[1] =nose_c;  // 0,1,2,3
            FN_cou++;
        }
        nose_c++;
    }
    // std::cout<<"1FN_cou: "<<FN_cou<<std::endl;
    // 用于记录眼睛眉毛点
    for(int i=51; i<70; i++){
        FN_key.row(FN_cou)[0] = distan.row(i)[1];
        FN_key.row(FN_cou)[1] =4;
        FN_cou++;
    }
    // std::cout<<"2FN_cou: "<<FN_cou<<std::endl;
    // 用于记录眼睛眉毛点
    for(int i=86; i<106; i++){
        FN_key.row(FN_cou)[0] = distan.row(i)[1];
        FN_key.row(FN_cou)[1] =5;
        FN_cou++;
    }
    // std::cout<<"3FN_cou: "<<FN_cou<<std::endl;
    // std::cout<<"FN_key: "<<FN_key.rows()<<std::endl;

    //------------------------鼻梁点分四个步骤完成-------------------------------------------
    // for(int i=0;i<key_mesh.rows(); i++){
    //     // std::cout<<"i="<<i<<std::endl;
    //     std::vector<FixedVertex> fixed_vertices;
    //     float curgrp = 0;
    //     int fixvec = key_mesh.row(i)[1]; 
    //     // std::cout<<"key_mesh:"<<key_mesh.row(i)[1]<<" \n";
    //     for(int k=12; k<51; k++){
    //         fixed_vertices.push_back({int(FN_key.row(k)[0]), 0}); // 加入锚点
    //     }
    //     for (int k=0 ; k < move_vec.cols(); k++) {
    //         // std::cout<<"k="<<k<<"  move_vec:"<<move_vec.row(i)[k]<<" \n";
    //         fixed_vertices.push_back({move_vec.row(i)[k], ++curgrp}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp
    //     }
    //     std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
    //     // std::cout<<" \n"<<std::endl;
    //     vertic_move_v1(mesh, fixed_vertices, fixvec, FN_key, 1); 
    // }
    //------------------------鼻梁点分四个步骤完成-------------------------------------------

    //------------------------鼻梁点分一个步骤完成（会存形变错误的情况，比如额头点会被拉伸回来）-------------------------------------------
    std::vector<FixedVertex> fixed_vertices;
    Eigen::Matrix<float, 4, 1> fix_mat;
    float curgrp = 0;
    // std::cout<<"key_mesh:"<<key_mesh.row(i)[1]<<" \n";
    // 将固定点保存到矩阵中
    for (int i=0;i<key_mesh.rows(); i++){
        fix_mat.row(i)[0] = key_mesh.row(i)[1];
    }
    // 添加锚点
    for(int k=12; k<51; k++){
        fixed_vertices.push_back({int(FN_key.row(k)[0]), 0}); // 加入锚点
    }
    
    for(int i=0;i<key_mesh.rows(); i++){
        for (int k=0 ; k < move_vec.cols(); k++) {
            fixed_vertices.push_back({move_vec.row(i)[k], ++curgrp}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp
        }   
    }

    std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
    vertic_move_v2(mesh, fixed_vertices, fix_mat, FN_key, 1); 
    //------------------------鼻梁点分一个步骤完成（会存形变错误的情况，比如额头点会被拉伸回来）-------------------------------------------
    //------------------------进行鼻梁自动移动变换----------------------------------------
    end = clock();
    std::cout<<"耗时 = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
    //-----------------------进行额头点修复---------------------------------
    return 0;
}

// 实现鼻子和人脸轮廓修复
int nose_face_main(int argc, char *argv[])
{
	clock_t start,end;
    start = clock(); 
    int image_width;
    //------------------------读取mesh点和keypoint点的code-----------------------
    std::string image_path = argv[1];
    std::string mesh_path="/Users/wpx/Documents/arap_v2/jsons/mesh_json.json";
    std::string keypoint_path="/Users/wpx/Documents/arap_v2/jsons/keypoint_json.json";
    std::string mesh_obj_path = "/Users/wpx/Documents/arap_v2/jsons/mesh.obj";
    std::string nose_obj_path = "/Users/wpx/Documents/arap_v2/build/examples/mesh_deformation.obj";
    std::string right_face_obj_path = "/Users/wpx/Documents/arap_v2/build/examples/face_mesh_deformation.obj";
    image_width = testVideo3D(image_path, mesh_path, keypoint_path, mesh_obj_path);  // image_width 这里表示从width 和height选择最小的值。
    //------------------------读取mesh点和keypoint点的code-----------------------

    //------------------------计算距离keypoing点最近的mesh点下标-----------------------
    std::string keyname = "keypoint";
    std::string meshname = "mesh";
    Eigen::MatrixX2f key2D;
    Eigen::MatrixX2f mesh2D;
    Eigen::MatrixX3f distan;
    key2D = readFileJson(keypoint_path, keyname);
    mesh2D = readFileJson(mesh_path, meshname);
    distan = computer_distance_v1(key2D, mesh2D, image_width);   // 返回的是【keypoint， mesh_point】,都表示下标，比如对于74号keypoint，其对应的mesh_point为2318.
    // std::cout<<"dis:"<<distan.row(74)<<std::endl;
    //------------------------计算距离keypoing点最近的mesh点下标-----------------------

    //------------------------进行鼻梁自动移动变换----------------------------------------
    std::cout<<"开始鼻子修复"<<std::endl;
    Mesh mesh;
   
    if (!load_model(mesh_obj_path, mesh)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    LaplacianSystem system;
    

    Eigen::Matrix<int, 4, 2> key_mesh;
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> move_vec;
    move_vec.resize(4,3);
    move_vec <<208, 931, 1803,  //对应71号点
                828, 923, 1276,  //对应72号点
                2412,2478,2276,  //对应73号点 
                2318, 2322,2318; //对应74号点
                // 2318, 2335,2322; //对应74号点
                             
    int j = 0;
    for(int i=71; i<75; i++){  //只获取鼻梁71到74号点最近的mesh点
        key_mesh.row(j)[0] = int(distan.row(i)[0]);
        key_mesh.row(j)[1] = int(distan.row(i)[1]);
        j++;
    }
    // std::cout<<"key_mesh_vet:"<<key_mesh<<std::endl;


    for(int i=0;i<key_mesh.rows(); i++){
        // std::cout<<"i="<<i<<std::endl;
        std::vector<FixedVertex> fixed_vertices;
        float curgrp = 0;
        int fixvec = key_mesh.row(i)[1]; 
        // std::cout<<"key_mesh:"<<key_mesh.row(i)[1]<<" \n";
        for (int k=0 ; k < move_vec.cols(); k++) {
            // std::cout<<"k="<<k<<"  move_vec:"<<move_vec.row(i)[k]<<" \n";
            fixed_vertices.push_back({move_vec.row(i)[k], ++curgrp}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp
        }
        std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
        // std::cout<<" \n"<<std::endl;
        vertic_move(mesh, fixed_vertices, fixvec, 1); 
    }
    
    // std::cout << mesh.V.rows() << ", " << elapsed.count() / iterations << std::endl;
   
    //------------------------进行鼻梁自动移动变换----------------------------------------

    //------------------------进行人脸轮廓（FA点在mesh外部）自动移动变换----------------------------------------
    std::cout<<"开始脸部轮廓修复"<<std::endl;
    Mesh meshs;
    // std::string pathsss="/Users/wpx/Documents/arap_v2/data/celianMeshMvp.obj";
    if (!load_model(nose_obj_path, meshs)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }

    //--------------------------------33个人脸点一起移动---------------------
    Eigen::Matrix<float, Eigen::Dynamic, 3> FM_key_mesh; 
    Eigen::Matrix<float, Eigen::Dynamic, 2> FM_key; 
    FM_key.resize(33+72, 2);
    FM_key_mesh.resize(33+72, 3);
    int fj = 0;
    for(int i=0; i<FM_key_mesh.rows(); i++){  //只获取鼻梁71到74号点最近的mesh点
        // if (int(distan.row(i)[2]) > 25){
            FM_key.row(fj)[0] = distan.row(i)[1];
            if (fj < 17){
                
                FM_key.row(fj)[1] = 0;
            }
            else if(fj>=17 && fj <33){
                FM_key.row(fj)[1] = 1;          
            }else{
                FM_key.row(fj)[1] = 2;
            }
            FM_key_mesh.row(fj)[0] = distan.row(i)[0];  // FA点
            FM_key_mesh.row(fj)[1] = distan.row(i)[1];  // 距离FA最近的Mesh点
            FM_key_mesh.row(fj)[2] = distan.row(i)[2];  // 距离FA最近Mesh点之间的距离
            fj++;

    }
    std::vector<FixedVertex> fixed_vertices;
    for(int i=0; i<FM_key_mesh.rows(); i++){
        if (i % 5 == 0){
            fixed_vertices.push_back({int(FM_key_mesh.row(i)[1]), FM_key_mesh.row(i)[2]}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp   
        }
    } 
    int fu = 0;
   
    std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
    Face_Contour_Restoration_v1(meshs, fixed_vertices, 1, fu, FM_key); 
    end = clock();  
    std::cout<<"耗时 = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
    //--------------------------------33个人脸点一起移动---------------------

    
    
    //------------------------进行人脸轮廓（FA点在mesh外部）自动移动变换----------------------------------------
    return 0;
}
// 实现额头点修复
int forehead_main(int argc, char *argv[]){
    clock_t start,end;
    start = clock(); 
    int image_width;
    std::string image_path = argv[1];
    std::string mesh_path="/Users/wpx/Documents/arap_v2/jsons/mesh_json.json";
    std::string key_path="/Users/wpx/Documents/arap_v2/jsons/keypoint_json.json";
    std::string keypoint_path=argv[2];
    std::string mesh_obj_path = "/Users/wpx/Documents/arap_v2/jsons/mesh.obj";
    std::string nose_obj_path = "/Users/wpx/Documents/arap_v2/build/examples/mesh_deformation.obj";
    std::string forehead_obj_path = "/Users/wpx/Documents/arap_v2/build/examples/forehead_mesh_deformation.obj";
    image_width = testVideo3D(image_path, mesh_path, key_path, mesh_obj_path);
    std::cout<<"读取图片完成"<<std::endl;
    //------------------------读取mesh点和keypoint点的code-----------------------

    //------------------------计算距离keypoing点最近的mesh点下标-----------------------
    std::string keyname = "forehead_lmk";
    std::string landmarks = "landmarks";
    std::string meshname = "mesh";
    Eigen::MatrixX2f key2D;
    Eigen::MatrixX2f mesh2D;
    Eigen::MatrixX3f distan;
    key2D = forehead_readFileJson(keypoint_path, keyname, landmarks);
    mesh2D = readFileJson(mesh_path, meshname);
    distan = computer_distance_v2(key2D, mesh2D, image_width);  
    std::cout<<"distan: \n"<<distan<<std::endl;

    std::cout<<"开始额头点修复"<<std::endl;
    Mesh meshes;
    // std::string pathsss="/Users/wpx/Documents/arap_v2/data/celianMeshMvp.obj";
    if (!load_model(mesh_obj_path, meshes)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    //--------------------------------33个人脸点一起移动---------------------
    Eigen::Matrix<float, Eigen::Dynamic, 3> FH_key_mesh; 
    Eigen::Matrix<float, Eigen::Dynamic, 2> FH_key; 
    FH_key.resize(87, 2);
    FH_key_mesh.resize(87, 3);
    int fj = 0;
    // 0:表示眉毛，眼睛，鼻梁点， 1:表示嘴巴点。 2:表示左额头点， 3:表示右额头点 
    for(int i=0; i<FH_key_mesh.rows(); i++){  //只获取鼻梁71到74号点最近的mesh点
        // if (int(distan.row(i)[2]) > 25){
            FH_key.row(fj)[0] = distan.row(i)[1];
            if (fj < 40){
                FH_key.row(fj)[1] = 0;      // 标记左轮廓点
            }else if(fj >=40 && fj < 60){
                FH_key.row(fj)[1] = 1;      // 标记左额头点
            }else if(fj >= 60 && fj < 73){
                FH_key.row(fj)[1] = 2;      // 标记右额头点
            }else{
                FH_key.row(fj)[1] = 3;
            }
            FH_key_mesh.row(fj)[0] = distan.row(i)[0];  // FA点
            FH_key_mesh.row(fj)[1] = distan.row(i)[1];  // 距离FA最近的Mesh点
            FH_key_mesh.row(fj)[2] = distan.row(i)[2];  // 距离FA最近Mesh点之间的距离
            fj++;

    }
    std::vector<FixedVertex> fixed_vertices;
    for(int i=0; i<FH_key_mesh.rows(); i++){

        fixed_vertices.push_back({int(FH_key_mesh.row(i)[1]), FH_key_mesh.row(i)[2]});
    } 
    int fu = 0;
   
    std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
    ForeFeadRestoration(meshes, fixed_vertices, 1, fu, FH_key); 
    end = clock();  
    std::cout<<"耗时 = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
    //--------------------------------33个人脸点一起移动---------------------
    return 0;
}

// 实现鼻子,人脸轮廓和额头点修复: nose_face_forehead
namespace fs = std::filesystem;  // 这个主要是为了获取当前目录的路径

int main(int argc, char *argv[])
{
	clock_t start,end;
    start = clock(); 
    Eigen::Matrix2Xf image_inf;
    std::cout<<"currend dir: "<<fs::current_path()<<std::endl;
    //------------------------读取mesh点和keypoint点的code-----------------------
    std::string image_path = argv[1];   // 输入的优化图片
    std::string forhead_keypoint_path=argv[2];  // 输入的额头点json文件
    std::string root = std::string(fs::current_path());  // 获取当前目录，并将其转换为string类型，方便之后与其他路径拼接
    std::string mesh_path = root + "/1_mesh_json.json";
    std::string keypoint_path= root + "/1_keypoint_json.json";
    std::string mesh_obj_path =  root + "/1_mesh.obj";
    std::string nose_obj_path = root + "/1_mesh_deformation.obj";
    std::string face_obj_path = root + "/1_face_mesh_deformation.obj";
    std::string forhead_obj_path =  root + "/1_ForeHeadRestoration.obj";
    image_inf = testVideo3D_v1(image_path, mesh_path, keypoint_path, mesh_obj_path);  // image_width 这里表示从width 和height选择最小的值。
    
    //------------------------读取mesh点和keypoint点的code-----------------------

    //------------------------计算距离keypoing点最近的mesh点下标-----------------------
    std::string forhead_keyname = "forehead_lmk";
    std::string forhead_landmarks = "landmarks";
    std::string keyname = "keypoint";
    std::string meshname = "mesh";
    Eigen::MatrixX2f FH_key2D;  // FH 表示 ForeHead
    Eigen::MatrixX2f key2D;
    Eigen::MatrixX2f mesh2D;
    Eigen::MatrixX3f FH_distan;
    double means;
    std::tuple<Eigen::MatrixX3f, double> distan_1;
    Eigen::MatrixX3f distan;
    FH_key2D = forehead_readFileJson(forhead_keypoint_path, forhead_keyname, forhead_landmarks); // 因为有已知的额头点关键点的json文件，所以在该文件中读取额头FA点。因此不参与mtface库生成过程FA过程
    key2D = readFileJson(keypoint_path, keyname);  // 需要mtface生成的关键点数据
    mesh2D = readFileJson(mesh_path, meshname);    // 读取生成的人脸mesh数据
    
    distan = computer_distance_v1(key2D, mesh2D, int(image_inf.row(1)[0]));   // 返回的是130个FA点最近mesh距离，其中0-33号点进行了归一化，用于计算权重。鼻梁点只需要找到最近的mesh点，不需要归一化。
    distan_1 = computer_distance_v3(FH_key2D, mesh2D, int(image_inf.row(1)[0]));   // 返回的是额头点计算的距离，其中还包括嘴巴，眼睛，眉毛，鼻梁处的锚点
    std::tie(FH_distan, means) = distan_1;

    // std::cout<<"means_v1   :"<<means<<std::endl;
    // std::cout<<"FH_distan:"<<FH_distan<<"\n ----------------\n";
    //------------------------计算距离keypoing点最近的mesh点下标-----------------------

    //------------------------进行鼻梁自动移动变换----------------------------------------
    std::cout<<"开始鼻子修复"<<std::endl;
    Mesh mesh;
   
    if (!load_model(mesh_obj_path, mesh)) {
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    LaplacianSystem system;
    

    Eigen::Matrix<int, 4, 2> key_mesh;
    Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> move_vec;
    move_vec.resize(4,3);
    move_vec <<208, 931, 1803,  //对应71号点
                828, 923, 1276,  //对应72号点
                2412,2478,2276,  //对应73号点 
                2318, 2322,2318; //对应74号点
                // 2318, 2335,2322; //对应74号点
                             
    int j = 0;
    int FN_cou = 0;
    Eigen::Matrix<float, Eigen::Dynamic, 2> FN_key; 
    FN_key.resize(12+19+20, 2);  //4表示鼻梁点，18表示眼睛点，20表示嘴巴点

    for(int i=71; i<75; i++){  //只获取鼻梁71到74号点最近的mesh点
        key_mesh.row(j)[0] = int(distan.row(i)[0]);
        key_mesh.row(j)[1] = int(distan.row(i)[1]);
        j++;
    }
    // 用于记录鼻子点
    int nose_c = 0;
    for(int i=0; i<move_vec.rows(); i++){
        for (int j=0; j <move_vec.cols(); j++){
            FN_key.row(FN_cou)[0] = move_vec.row(i)[j];
            FN_key.row(FN_cou)[1] =nose_c;  // 0,1,2,3
            FN_cou++;
        }
        nose_c++;
    }
    // std::cout<<"1FN_cou: "<<FN_cou<<std::endl;
    // 用于记录眼睛眉毛点
    for(int i=51; i<70; i++){
        FN_key.row(FN_cou)[0] = distan.row(i)[1];
        FN_key.row(FN_cou)[1] =4;
        FN_cou++;
    }
    // std::cout<<"2FN_cou: "<<FN_cou<<std::endl;
    // 用于记录眼睛眉毛点
    for(int i=86; i<106; i++){
        FN_key.row(FN_cou)[0] = distan.row(i)[1];
        FN_key.row(FN_cou)[1] =5;
        FN_cou++;
    }
    // std::cout<<"3FN_cou: "<<FN_cou<<std::endl;
    // std::cout<<"FN_key: "<<FN_key.rows()<<std::endl;

   
    for(int i=0;i<key_mesh.rows(); i++){
        // std::cout<<"i="<<i<<std::endl;
        std::vector<FixedVertex> fixed_vertices;
        float curgrp = 0;
        int fixvec = key_mesh.row(i)[1]; 
        // std::cout<<"key_mesh:"<<key_mesh.row(i)[1]<<" \n";
        for(int k=12; k<51; k++){
            fixed_vertices.push_back({int(FN_key.row(k)[0]), 0}); // 加入锚点
        }
        for (int k=0 ; k < move_vec.cols(); k++) {
            // std::cout<<"k="<<k<<"  move_vec:"<<move_vec.row(i)[k]<<" \n";
            fixed_vertices.push_back({move_vec.row(i)[k], ++curgrp}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp
        }
        std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
        // std::cout<<" \n"<<std::endl;
        vertic_move_v1(mesh, fixed_vertices, fixvec, FN_key, 1); 
    }
   
    //------------------------进行鼻梁自动移动变换----------------------------------------

    //------------------------进行人脸轮廓（FA点在mesh外部）自动移动变换----------------------------------------
    std::cout<<"开始脸部轮廓修复"<<std::endl;
    Mesh meshs;
    // std::string pathsss="/Users/wpx/Documents/arap_v2/data/celianMeshMvp.obj";
    if (!load_model(nose_obj_path, meshs)) {  // 加载鼻梁修复后的obj文件，用于人脸轮廓修复
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }

    //--------------------------------33个人脸点一起移动---------------------
    Eigen::Matrix<float, Eigen::Dynamic, 3> FM_key_mesh; 
    Eigen::Matrix<float, Eigen::Dynamic, 2> FM_key; 
    FM_key.resize(33+72, 2);  // 其中33表示人脸轮廓点，72表示眉毛+眼睛+鼻子+嘴巴点
    FM_key_mesh.resize(33+72, 3);
    int fj = 0;
    for(int i=0; i<FM_key_mesh.rows(); i++){  //只获取鼻梁71到74号点最近的mesh点
        // if (int(distan.row(i)[2]) > 25){
            FM_key.row(fj)[0] = distan.row(i)[1];
            if (fj < 17){
                
                FM_key.row(fj)[1] = 0;
            }
            else if(fj>=17 && fj <33){
                FM_key.row(fj)[1] = 1;          
            }else{
                FM_key.row(fj)[1] = 2;
            }
            FM_key_mesh.row(fj)[0] = distan.row(i)[0];  // FA点
            FM_key_mesh.row(fj)[1] = distan.row(i)[1];  // 距离FA最近的Mesh点
            FM_key_mesh.row(fj)[2] = distan.row(i)[2];  // 距离FA最近Mesh点之间的距离
            fj++;

    }

    std::vector<FixedVertex> fixed_vertices;
    for(int i=0; i<FM_key_mesh.rows(); i++){
        if (i % 3 == 0){
            fixed_vertices.push_back({int(FM_key_mesh.row(i)[1]), FM_key_mesh.row(i)[2]}); //将需要拟合的关键点放入 fixed_vertices 栈中。不可移动的点curgrp=0.可移动的>0；并且group==curgrp   
        }
    } 
    int fu = 0;
   
    std::sort(fixed_vertices.begin(), fixed_vertices.end(), compare_by_index);
    Face_Contour_Restoration_v2(meshs, fixed_vertices, 1, fu, FM_key, image_inf.row(0)[0]); 
    // end = clock();  
    // std::cout<<"耗时 = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
    //--------------------------------33个人脸点一起移动---------------------

    //------------------------进行人脸轮廓（FA点在mesh外部）自动移动变换----------------------------------------
    
    
    //-----------------------进行额头点修复-------------------------------
    std::cout<<"开始额头点修复"<<std::endl;
    Mesh meshes;
    if (!load_model(face_obj_path, meshes)) { // 加载人脸轮廓修复后的obj文件，用于额头点修复
        std::cerr << "Could not load model." << std::endl;
        return 1;
    }
    Eigen::Matrix<float, Eigen::Dynamic, 3> FH_key_mesh; 
    Eigen::Matrix<float, Eigen::Dynamic, 2> FH_key; 
    FH_key.resize(87, 2);
    FH_key_mesh.resize(87, 3);
    fj = 0;
    int FH_index = 0;
    // 100:表示眉毛，眼睛，鼻梁点， 111:表示嘴巴点。 2:表示左额头点， 3:表示右额头点 
    for(int i=0; i<FH_key_mesh.rows(); i++){  //只获取鼻梁71到74号点最近的mesh点
        // if (int(FH_distan.row(i)[2]) > 25){
            FH_key.row(fj)[0] = FH_distan.row(i)[1];
            if (fj < 40){
                FH_key.row(fj)[1] = 100;      
            }else if(fj >=40 && fj < 60){
                FH_key.row(fj)[1] = 111;      
            }else if(fj >= 60 && fj < 73){
                FH_key.row(fj)[1] = FH_index++;      // 标记左额头点 0-13
            }else{
                FH_key.row(fj)[1] = FH_index++;    // 标记右额头点 13-27
            }
            FH_key_mesh.row(fj)[0] = FH_distan.row(i)[0];  // FA点
            FH_key_mesh.row(fj)[1] = FH_distan.row(i)[1];  // 距离FA最近的Mesh点
            FH_key_mesh.row(fj)[2] = FH_distan.row(i)[2];  // 距离FA最近Mesh点之间的距离
            fj++;

    }
    std::vector<FixedVertex> forhead_fixed_vertices;
    for(int i=0; i<FH_key_mesh.rows(); i++){
        forhead_fixed_vertices.push_back({int(FH_key_mesh.row(i)[1]), FH_key_mesh.row(i)[2]});
    } 
    fu = 0;
    
    std::sort(forhead_fixed_vertices.begin(), forhead_fixed_vertices.end(), compare_by_index);
    ForeFeadRestoration_v1(meshes, forhead_fixed_vertices, 1, fu, FH_key, image_inf, means);
    // ForeFeadRestoration(meshes, forhead_fixed_vertices, 1, fu, FH_key);
    end = clock();
    std::cout<<"耗时 = "<<double(end-start)/CLOCKS_PER_SEC<<"s"<<std::endl;
    //-----------------------进行额头点修复---------------------------------
    return 0;
}