#include "arap.hpp"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <algorithm> // std::sort

#define CLAMP_WIJ_TO_ZERO

static double compute_area(const Mesh& mesh) {
    double area = 0.0;
    for (int fid = 0; fid < mesh.F.rows(); fid++) {
        Eigen::Vector3d e1 = mesh.V.row(mesh.F(fid, 1)) - mesh.V.row(mesh.F(fid, 0));
        Eigen::Vector3d e2 = mesh.V.row(mesh.F(fid, 2)) - mesh.V.row(mesh.F(fid, 0));

	area += e1.cross(e2).norm();
    }
    return area / 2.;
}

Eigen::SparseMatrix<double> cotangent_weights(const Mesh& mesh, const std::vector<Eigen::Index>& swizzle) {
    Eigen::SparseMatrix<double> weights(mesh.V.rows(), mesh.V.rows());
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(18 * mesh.F.rows());

    for (int fid = 0; fid < mesh.F.rows(); fid++) {
        Eigen::Vector3i v = mesh.F.row(fid);  // 当前的下标为fid的face值

        Eigen::Matrix<double, 3, 3> edges;
        for (int j = 0; j < 3; j++) {
            int k = (j + 1) % 3; // 1，2，0

            // edge vector between v(j) and v(k)
            //1-0； 2-1；0-2
            edges.row(j) = mesh.V.row(v(k)) - mesh.V.row(v(j)); //最终得到3x3的矩阵，第一行代表1-0顶点的差值，是一个一行三列的值。以此类推。
        }

        double area_doubled = edges.row(0).cross(edges.row(1)).norm(); //计算叉乘在做向量的二范数。 最终得到一个double类型的值。
        double one_over_8area = 1.0 / (4 * area_doubled);
        /*
            上面两行计算了总的一个权重值（我觉得可以理解为cell的权重值），下面首先计算每一个边的权重值，然后与cell相乘得到当前face某两个值的权重，然后
            利用三元组存储。（毕竟三元组是可以大大减少存储空间），之后放入栈中。
        */
        for (int j = 0; j < 3; j++) {
            int k = (j + 1) % 3; // 1，2，0
            int i = (j + 2) % 3; // 2， 0， 1
            
            double d2 = edges.row(j).squaredNorm();  // squaredNorm:是norm的根，即没有开二次根号。

            int ci = swizzle[v(i)]; // 2， 0， 1
            int cj = swizzle[v(j)]; // 0， 1， 2
            int ck = swizzle[v(k)]; // 1， 2， 0

            double contribution = one_over_8area * d2;

            triplets.push_back(Eigen::Triplet<double>(cj, ck, -contribution));
            triplets.push_back(Eigen::Triplet<double>(ck, cj, -contribution));
            
            triplets.push_back(Eigen::Triplet<double>(ci, cj, contribution));
            triplets.push_back(Eigen::Triplet<double>(cj, ci, contribution));
            
            triplets.push_back(Eigen::Triplet<double>(ci, ck, contribution));
            triplets.push_back(Eigen::Triplet<double>(ck, ci, contribution));
        }

    }
    // 聚合三元组中重复的值。使用了DupFunctor函数，不太理解怎么处理的，难道删除了？
    weights.setFromTriplets(triplets.begin(), triplets.end());

//将某些边的权重低于0的直接赋予0，说明这些边并没有参与变换
#ifdef CLAMP_WIJ_TO_ZERO

    // weights:应该类似与于这种结构；（x,a）,(y,b),(z,c),,
    // weights.outerSize():如果原始矩阵是3x，那么这个值应该等于列数。
    for (int k=0; k < weights.outerSize(); ++k) {
	double colsum = 0;
	for (Eigen::SparseMatrix<double>::InnerIterator it(weights,k); it; ++it) {
        // 应该还可以查到另外三种值，it的行数row，列数col以及下标index。
	    if (it.value() < 0.0) {
        // coedRef:利用二分查找，较为耗时，
		weights.coeffRef(it.row(), it.col()) = 0.0;
		// std::cout << "[WARNING] Ignoring edge " << it.row() << " - " << it.col() << "value:"<<it.value()<<"\n";
	    }
        // std::cout << "[WARNING] Ignoring edge " << it.row() << " - " << it.col() << "value:"<<it.value()<<"\n";
	}
    }
#endif

    return weights;
}

Eigen::SparseMatrix<double> laplacian_matrix(
    const Eigen::SparseMatrix<double>& weights) {
    
    Eigen::SparseMatrix<double> mat = -weights;

    for (int k=0; k < mat.outerSize(); ++k) {
        double colsum = 0;
        for (Eigen::SparseMatrix<double>::InnerIterator it(mat,k); it; ++it) {
            colsum += it.value();
        }
        mat.coeffRef(k, k) = -colsum; //对角线赋予权重，假设是三维矩阵，那么就是将每一列的权重值累加并取负，之后赋予到对角线上。
    }

    return mat;
}

Eigen::Matrix3d compute_best_rotation(const LaplacianSystem& system, int r) {
    Eigen::Matrix3d cov = Eigen::Matrix3d::Zero();

    for (Eigen::SparseMatrix<double>::InnerIterator it(system.cotangent_weights, r); it; ++it) {
	Eigen::Index v_idx[2] = {
	    system.deswizzle[it.col()],
	    system.deswizzle[it.row()]
	};

	Eigen::Vector3d e = system.mesh->V.row(v_idx[0]) - system.mesh->V.row(v_idx[1]);
	Eigen::Vector3d e0 = system.V0.row(v_idx[0]) - system.V0.row(v_idx[1]);

	cov += it.value() * (e0 * e.transpose() +
			     system.rotation_variation_penalty * system.optimal_rotations[it.row()].transpose());
    }

    Eigen::JacobiSVD<Eigen::Matrix3d> svd(cov, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::Matrix3d um = svd.matrixU();
    Eigen::Matrix3d vm = svd.matrixV();
    Eigen::Matrix3d rot = vm * um.transpose();

    if (rot.determinant() < 0) {
	um.col(2) *= -1;
	rot = vm * um.transpose();
    }

    assert(fabs(rot.determinant() - 1.0) < 1e-3);

    return rot;
}

std::vector<Eigen::Index> swizzle_from(size_t n, const std::vector<FixedVertex>& fixed_vertices) {
    std::vector<Eigen::Index> swizzled(n);  // 生成n行，值为0；
    
    size_t free_offset = 0;
    size_t fixed_offset = n - fixed_vertices.size();

    size_t swizzled_offset = 0;
    
    // 确定选点关键点的位置。并赋予 fixed_offset 到 n的值到相应的位置
    for (int i = 0; i < fixed_vertices.size(); i++) {
        // std::cout<<"index:"<<fixed_vertices[i].group<<std::endl;
        for (; swizzled_offset < fixed_vertices[i].index; swizzled_offset++) {
            swizzled[swizzled_offset] = free_offset++;
        }
        swizzled[swizzled_offset] = fixed_offset++;
        swizzled_offset++;
    }
    
    /*
        如果关键点为：0，1，2，3。总的关键点为3520。则swizzled数组为[3516, 3517, 3518,3519, 0, 1, 2, 3,...,3515]
    */
    for (; swizzled_offset < n; swizzled_offset++) {
	swizzled[swizzled_offset] = free_offset++;
    }
    // for(int i=0;i<20;i++){
    //     std::cout<<"swizzled 2      :"<<swizzled[i]<<std::endl;   // swizzled       :3516
    // }
    return swizzled;
}

std::vector<Eigen::Index> reciprocal(const std::vector<Eigen::Index>& v) {
    std::vector<Eigen::Index> r(v.size());
    // 将关键点的位置对应的下标移到最后几位，有n个关键点，后面几位就是的排列就是0-n；
    /*
        如果关键点为：0，1，2，3。总的关键点为3520。则r数组为[4,5,6...,3518,3519, 0, 1, 2, 3]
    */
    for (int i = 0; i < v.size(); i++) {
	r[v[i]] = i;
    }

    
    return r;
}

void system_init(LaplacianSystem& system, Mesh* mesh, double alpha) {
    system.mesh = mesh;
    system.is_bound = false;
    system.iterations = 0;

    system.rotation_variation_penalty = alpha * compute_area(*mesh);

    system.optimal_rotations.reserve(mesh->V.rows());
    for (size_t i = 0; i < mesh->V.rows(); i++) {
	    system.optimal_rotations.push_back(Eigen::Matrix3d::Identity());
    }
}

// 
bool system_bind(LaplacianSystem& system, const std::vector<FixedVertex>& fixed_vertices) {
    system.V0 = system.mesh->V;
    system.is_bound = true;
    std::cout<<"system_bind------:"<<fixed_vertices.size()<<std::endl;
    // for (const auto& vertex : fixed_vertices) {
    //     std::cout<<"--------------index:------:"<<vertex.index <<"\n";
    //     std::cout<<"before vertex.index:------:"<<system.mesh->V.row(vertex.index) <<"\n";
				
	// }
    system.free_dimension = system.mesh->V.rows() - fixed_vertices.size();
    
    system.swizzle = swizzle_from(system.mesh->V.rows(), fixed_vertices);
    system.deswizzle = reciprocal(system.swizzle);
	//计算边的权重，其实也计算了每个cell的权重
    system.cotangent_weights = cotangent_weights(*system.mesh, system.swizzle);
    //只有对角线的值>0,其他位置<0;
    Eigen::SparseMatrix<double> m = laplacian_matrix(system.cotangent_weights); 
    // 得到一个system.free_dimension行， system.free_dimension列的laplacian矩阵
    system.laplacian_matrix = m.block(0, 0, system.free_dimension, system.free_dimension);
    // 从0行system.free_dimension列出发，返回大小为system.free_dimension行, fixed_vertices.size()列的矩阵。
    system.fixed_constraint_matrix = m.block(0, system.free_dimension,
					     system.free_dimension, fixed_vertices.size());
    
    system.rhs.resize(system.free_dimension, 3);
    // 计算厄米特矩阵（Hermitian矩阵）。
    // Hermitian矩阵：矩阵中每一个第i行第j列的元素都与第j行第i列的元素的共轭相等。埃尔米特矩阵主对角线上的元素都是实数的，其特征值也是实数。
    system.solver.compute(system.laplacian_matrix); // 先对矩阵进行分解，然后求解线性方程组的解。
    /*
        在compute()函数中，将对矩阵进行分解。为了更加精确的求解，compute ()函数计算步骤又进一步细分为2步：
            analyzePattern()：记录矩阵中的非零元素，以便分解步骤中创建更少的fill-in
            factorize()：计算系数矩阵的因子。只要矩阵的值发生变化，就需要调用该函数。然而在多次调用之间，应该保证矩阵的结构不变化
        solve()函数计算线性方程组的解。等号右边可以是一个向量，也可以是多个向量（矩阵）
    */
    if (system.solver.info() != Eigen::Success) {
        // solve failed， 没有计算出解
        // std::cout<<"solve filed"<<std::endl;
	    return false;
    }

    // std::cout << "Cotangent weights :\n" << system.cotangent_weights << "\n";
    // std::cout << "Laplacian matrix :\n" << system.laplacian_matrix << "\n";
    // std::cout << "A matrix :\n" << system.fixed_constraint_matrix << "\n";
    
    return true;
}

bool system_bind1(LaplacianSystem& system, const std::vector<FixedVertex>& fixed_vertices) {
    system.V0 = system.mesh->V;
    system.is_bound = true;
    // std::cout<<"system_bind:"<<fixed_vertices[0].index<<std::endl;
    
    system.free_dimension = system.mesh->V.rows() - fixed_vertices.size();
    
    system.swizzle = swizzle_from(system.mesh->V.rows(), fixed_vertices);
    system.deswizzle = reciprocal(system.swizzle);
	//计算边的权重，其实也计算了每个cell的权重
    system.cotangent_weights = cotangent_weights(*system.mesh, system.swizzle);
    //只有对角线的值>0,其他位置<0;
    Eigen::SparseMatrix<double> m = laplacian_matrix(system.cotangent_weights); 
    // 得到一个system.free_dimension行， system.free_dimension列的laplacian矩阵
    system.laplacian_matrix = m.block(0, 0, system.free_dimension, system.free_dimension);
    // 从0行system.free_dimension列出发，返回大小为system.free_dimension行, fixed_vertices.size()列的矩阵。
    system.fixed_constraint_matrix = m.block(0, system.free_dimension,
					     system.free_dimension, fixed_vertices.size());
    
    system.rhs.resize(system.free_dimension, 3);
    // 计算厄米特矩阵（Hermitian矩阵）。
    // Hermitian矩阵：矩阵中每一个第i行第j列的元素都与第j行第i列的元素的共轭相等。埃尔米特矩阵主对角线上的元素都是实数的，其特征值也是实数。
    system.solver.compute(system.laplacian_matrix); // 先对矩阵进行分解，然后求解线性方程组的解。
    /*
        在compute()函数中，将对矩阵进行分解。为了更加精确的求解，compute ()函数计算步骤又进一步细分为2步：
            analyzePattern()：记录矩阵中的非零元素，以便分解步骤中创建更少的fill-in
            factorize()：计算系数矩阵的因子。只要矩阵的值发生变化，就需要调用该函数。然而在多次调用之间，应该保证矩阵的结构不变化
        solve()函数计算线性方程组的解。等号右边可以是一个向量，也可以是多个向量（矩阵）
    */
    if (system.solver.info() != Eigen::Success) {
        // solve failed， 没有计算出解
        // std::cout<<"solve filed"<<std::endl;
	    return false;
    }

    // std::cout << "Cotangent weights :\n" << system.cotangent_weights << "\n";
    // std::cout << "Laplacian matrix :\n" << system.laplacian_matrix << "\n";
    // std::cout << "A matrix :\n" << system.fixed_constraint_matrix << "\n";
    
    return true;
}

bool system_iterate(LaplacianSystem& system) {
    /* --- Compute approximate rotations --- */
    
    // for (int i = 0; i < system.mesh->V.rows(); i++) {
	//     system.optimal_rotations[i] = compute_best_rotation(system, i);
    // }

    /* --- Fill system's right hand side --- */
    
    system.rhs.setZero();

    for (int v = 0; v < system.free_dimension; v++) {
	for (Eigen::SparseMatrix<double>::InnerIterator
		 it(system.cotangent_weights, v);
	     it;
	     ++it) {
        
	    Eigen::Index v_idx[2] = {
		system.deswizzle[it.col()],
		system.deswizzle[it.row()],
	    };
        // std::cout<<"row::"<<it.row()<<"  col:"<<it.col()<<"  value:"<<it.value()<<"\n";
        // std::cout<<"v_idx[0]::"<<v_idx[0]<<"  v_idx[1]:"<<v_idx[1]<<"\n";
	    Eigen::RowVector3d d = .5 * it.value() *
		(system.V0.row(v_idx[0]) - system.V0.row(v_idx[1])) *
		(system.optimal_rotations[it.row()] + system.optimal_rotations[it.col()]).transpose();

	    system.rhs.row(v) += d;
        // std::cout<<"deformation:"<<system.rhs.row(v)<<"\n";
	} 
    } 

    int n_fixed = system.mesh->V.rows() - system.free_dimension;
    Eigen::Matrix<double, Eigen::Dynamic, 3>
	V_fixed(n_fixed, 3);
    
    for (int i = 0; i < n_fixed; i++) {
        // std::cout<<"V_fixed.row(i) :"<<V_fixed.row(i)<<"   deswizzle:"<<system.mesh->V.row(system.deswizzle[system.free_dimension + i])<<"\n";
	V_fixed.row(i) =
	    system.mesh->V.row(system.deswizzle[system.free_dimension + i]);
    }

    system.rhs -= system.fixed_constraint_matrix * V_fixed;
    
    Eigen::Matrix<double, Eigen::Dynamic, 3> solutions(system.free_dimension, 3);
    // std::cout<<"system.free_dimension:"<<solutions(system.free_dimension, 3)<<"\n";
    for (int i = 0; i < 3; i++) {
	solutions.col(i) = system.solver.solve(system.rhs.col(i));
    
	
	if (system.solver.info() != Eigen::Success) {
	    return false;
	}
    }
    assert((system.laplacian_matrix * solutions
	    - system.rhs).norm() < 1e-3);

    system.mesh_access.lock();
    for (int i = 0; i < system.free_dimension; i++) {
	system.mesh->V.row(system.deswizzle[i])
	    = solutions.row(i);
        
    }
    system.mesh_access.unlock();

    system.iterations++;

    return true;
}

void system_solve(LaplacianSystem& system, int iterations) {
    for (int i = 0; i < iterations; i++) {
	    system_iterate(system);
    }
}
