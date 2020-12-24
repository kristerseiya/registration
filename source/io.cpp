
#include "io.h"

#include "pointcloud.h"

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <Eigen/Core>
#include <Eigen/StdVector>

void ReadDataFromFile(const std::string& filename, std::vector<Eigen::Vector3d>& points,
                      std::vector<Eigen::Vector3d>& normals,
                      std::vector<Eigen::Vector3d>& colors) {
  if (filename.compare(filename.size()-4,4,".stl")==0) {
    ReadSTL(filename, points, normals, colors);
  } else if (filename.compare(filename.size()-5,5,".xyzm")==0) {
    ReadXYZM(filename, points, normals, colors);
  } else if (filename.compare(filename.size()-4,4,".pts")==0) {
    ReadPCD(filename, points, normals, colors);
  } else if (filename.compare(filename.size()-4,4,".pcd")==0) {
    ReadPCD(filename, points, normals, colors);
  } else {
    fprintf(stderr,"could not recognize file extension\n");
    exit(1);
  }
  return;
}

void ReadSTL(const std::string& filename, std::vector<Eigen::Vector3d>& points,
             std::vector<Eigen::Vector3d>& normals,
             std::vector<Eigen::Vector3d>& colors) {

  FILE* fp = fopen(filename.c_str(),"rb");
  if (fp == NULL) {
    printf("failed to open file\n");
    exit(1);
  }
  char header[80];
  fread(header,sizeof(char),80,fp);
  unsigned int n_face;
  fread(&n_face, sizeof(unsigned int), 1, fp);
  size_t n_points = ((size_t)n_face) * 3;

  points.clear();
  normals.clear();
  colors.clear();
  points.resize(n_points);

  float buffer[9];
  for (size_t i = 0; i < n_face; i++) {
    fread(buffer, sizeof(float), 3, fp);
    fread(buffer, sizeof(float), 9, fp);
    points[i*3+0] = Eigen::Vector3d(buffer[0], buffer[1], buffer[2]);
    points[i*3+1] = Eigen::Vector3d(buffer[3], buffer[4], buffer[5]);
    points[i*3+2] = Eigen::Vector3d(buffer[6], buffer[7], buffer[8]);
    fread(buffer, sizeof(short), 1, fp);
  }

  fclose(fp);

  return;
}

void ReadPCD(const std::string& filename, std::vector<Eigen::Vector3d>& points,
             std::vector<Eigen::Vector3d>& normals,
             std::vector<Eigen::Vector3d>& colors) {

  FILE* fp = fopen(filename.c_str(), "rb");
  if (fp == NULL) {
    fprintf(stderr, "Could not read file %s\n", filename.c_str());
    exit(1);
  }

  unsigned long size;
  fread(&size, sizeof(unsigned long), 1, fp);

  points.clear();
  normals.clear();
  colors.clear();
  points.resize(size);

  float buffer[3];
  for (unsigned long i = 0; i < size; i++) {
    fread(buffer, sizeof(float), 3, fp);
    points[i] = Eigen::Vector3d(buffer[0],buffer[1],buffer[2]);
  }

  if (fread(buffer, sizeof(float), 3, fp)==3) {
    colors.resize(size);
    colors[0] = Eigen::Vector3d(buffer[0],buffer[1],buffer[2]);
    for (unsigned long i = 1; i < size; i++) {
      fread(buffer, sizeof(float), 3, fp);
      colors[i] = Eigen::Vector3d(buffer[0],buffer[1],buffer[2]);
    }
  }

  fclose(fp);

  return;
}

void WritePCD(const PointCloud& pcd, const std::string& filename) {
  FILE* fp = fopen(filename.c_str(), "wb");
  unsigned long size = pcd.points_.size();
  fwrite(&size, sizeof(unsigned long), 1, fp);
  float buffer[3];
  for (auto point : pcd.points_) {
	  buffer[0] = point[0]; buffer[1] = point[1]; buffer[2] = point[2];
    fwrite(buffer, sizeof(float), 3, fp);
  }
  for (auto color : pcd.colors_) {
    buffer[0] = color[0]; buffer[1] = color[1]; color[2] = buffer[2];
    fwrite(buffer, sizeof(float), 3, fp);
  }
  fclose(fp);
  return;
}

//
// void read_ply(char* path, size_t* n_points, float* points, float* normals, unsigned char* colors) {
//
//   char line[101];
//   float pt[3];
//   float n_pt[3];
//   unsigned char color[3];
//
//   FILE* ply_file = fopen(path,"rb");
//
//   if (ply_file == NULL) {
//     printf("failed to open file\n");
//     exit(1);
//   }
//
//   for (int i = 0; i < 13; i++) {
//     fgets(line,100,ply_file);
//   }
//
//   *n_points = 157006;
//   points = new float[3* (*n_points)];
//   normals = new float[3*(*n_points)];
//   colors = new unsigned char[3*(*n_points)];
//
//   size_t i = 0;
//   while (!feof(ply_file)) {
//     fread(pt,sizeof(float),3,ply_file);
//     fread(n_pt,sizeof(float),3,ply_file);
//     fread(color,sizeof(unsigned char),3,ply_file);
//
//     if (!feof(ply_file)) {
//       unsigned char bytes[4];
//       for (int i = 0; i < 3; i++) {
//         memcpy(bytes,&pt[i],4);
//         int big_endian = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
//         memcpy(&pt[i],&big_endian,4);
//         memcpy(bytes,&(n_pt[i]),4);
//         big_endian = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
//         memcpy(&n_pt[i],&big_endian,4);
//       }
//       memcpy(points+3*i,pt,3*sizeof(float));
//       memcpy(normals+3*i,n_pt,3*sizeof(float));
//       memcpy(colors+3*i,color,3*sizeof(unsigned char));
//       i++;
//     }
//   }
//
//   fclose(ply_file);
//
// }

void ReadXYZM(const std::string& filename, std::vector<Eigen::Vector3d>& points,
              std::vector<Eigen::Vector3d>& normals,
              std::vector<Eigen::Vector3d>& colors) {

  FILE* fp = fopen(filename.c_str(),"rb");
  if (fp == NULL) {
    fprintf(stderr,"failed to open file\n");
    exit(1);
  }
  int height;
  int width;
  int ret = fscanf(fp, "image size width x height = %d x %d", &width, &height);
  if (ret!=2) {
    printf("unsupported file format\n");
    fclose(fp);
    exit(1);
  }
  while(fgetc(fp)==0);
  fseek(fp,-1,SEEK_CUR);
  points.clear();
  normals.clear();
  colors.clear();
  points.resize(width*height);
  float buffer[3];
  for (size_t i = 0; i < (size_t)width*height; i++) {
    fread(buffer, sizeof(float), 3, fp);
    points[i] = Eigen::Vector3d(buffer[0], buffer[1], buffer[2]);
  }
  fclose(fp);

  return;
}
