#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION 
#endif

#ifndef FLOATDEF
#define FLOATDEF
#ifdef RAY_FLOAT_AS_DOUBLE
typedef double Float;
#else
typedef float Float;
#endif 
#endif

#include "vec3.h"
#include "vec2.h"
#include "point3.h"
#include "point2.h"
#include "normal.h" 
#include "mathinline.h"
#include "transform.h"
#include "transformcache.h"
#include "camera.h"
#include "float.h"
#include "buildscene.h"
#include "RProgress.h"
#include "rng.h"
#include "tonemap.h"
#include "infinite_area_light.h"
#include "adaptivesampler.h"
#include "sampler.h"
#include "color.h"
#include "integrator.h"
#include "debug.h"
using namespace Rcpp;
// [[Rcpp::plugins(cpp11)]]
// [[Rcpp::depends(RcppThread)]]
#include "RcppThread.h"

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#include <fstream>
#endif

using namespace std;

// [[Rcpp::export]]
List render_scene_rcpp(List camera_info, List scene_info) {
  
  //Unpack scene info
  bool ambient_light = as<bool>(scene_info["ambient_light"]);
  IntegerVector type = as<IntegerVector>(scene_info["type"]);
  NumericVector radius = as<NumericVector>(scene_info["radius"]);
  IntegerVector shape = as<IntegerVector>(scene_info["shape"]);
  List position_list = as<List>(scene_info["position_list"]);
  List properties = as<List>(scene_info["properties"]);
  int n = as<int>(scene_info["n"]);
  NumericVector bghigh  = as<NumericVector>(scene_info["bghigh"]);
  NumericVector bglow = as<NumericVector>(scene_info["bglow"]);
  LogicalVector ischeckered = as<LogicalVector>(scene_info["ischeckered"]);
  List checkercolors = as<List>(scene_info["checkercolors"]);
  List gradient_info = as<List>(scene_info["gradient_info"]);
  NumericVector noise = as<NumericVector>(scene_info["noise"]);
  LogicalVector isnoise = as<LogicalVector>(scene_info["isnoise"]);
  NumericVector noisephase = as<NumericVector>(scene_info["noisephase"]);
  NumericVector noiseintensity = as<NumericVector>(scene_info["noiseintensity"]);
  List noisecolorlist = as<List>(scene_info["noisecolorlist"]);
  List angle = as<List>(scene_info["angle"]);
  LogicalVector isimage = as<LogicalVector>(scene_info["isimage"]);
  CharacterVector filelocation = as<CharacterVector>(scene_info["filelocation"]);
  List alphalist = as<List>(scene_info["alphalist"]);
  NumericVector lightintensity = as<NumericVector>(scene_info["lightintensity"]);
  LogicalVector isflipped = as<LogicalVector>(scene_info["isflipped"]);
  LogicalVector isvolume = as<LogicalVector>(scene_info["isvolume"]);
  NumericVector voldensity = as<NumericVector>(scene_info["voldensity"]);
  LogicalVector implicit_sample = as<LogicalVector>(scene_info["implicit_sample"]);
  List order_rotation_list = as<List>(scene_info["order_rotation_list"]);
  float clampval = as<float>(scene_info["clampval"]);
  LogicalVector isgrouped = as<LogicalVector>(scene_info["isgrouped"]);
  List group_transform = as<List>(scene_info["group_transform"]);
  LogicalVector tri_normal_bools = as<LogicalVector>(scene_info["tri_normal_bools"]);
  LogicalVector is_tri_color = as<LogicalVector>(scene_info["is_tri_color"]);
  List tri_color_vert = as<List>(scene_info["tri_color_vert"]);
  CharacterVector fileinfo = as<CharacterVector>(scene_info["fileinfo"]);
  CharacterVector filebasedir = as<CharacterVector>(scene_info["filebasedir"]);
  bool progress_bar = as<bool>(scene_info["progress_bar"]);
  int numbercores = as<int>(scene_info["numbercores"]);
  bool hasbackground = as<bool>(scene_info["hasbackground"]);
  CharacterVector background = as<CharacterVector>(scene_info["background"]);
  List scale_list = as<List>(scene_info["scale_list"]);
  NumericVector sigmavec = as<NumericVector>(scene_info["sigmavec"]);
  float rotate_env = as<float>(scene_info["rotate_env"]);
  float intensity_env = as<float>(scene_info["intensity_env"]);
  bool verbose = as<bool>(scene_info["verbose"]);
  int debug_channel = as<int>(scene_info["debug_channel"]);
  IntegerVector shared_id_mat = as<IntegerVector>(scene_info["shared_id_mat"]);
  LogicalVector is_shared_mat = as<LogicalVector>(scene_info["is_shared_mat"]);
  float min_variance = as<float>(scene_info["min_variance"]);
  int min_adaptive_size = as<int>(scene_info["min_adaptive_size"]);
  List glossyinfo = as<List>(scene_info["glossyinfo"]);
  List image_repeat = as<List>(scene_info["image_repeat"]);
  List csg_info = as<List>(scene_info["csg_info"]);
  List mesh_list = as<List>(scene_info["mesh_list"]);
  List roughness_list = as<List>(scene_info["roughness_list"]);
  List animation_info = as<List>(scene_info["animation_info"]);
  

  
  auto startfirst = std::chrono::high_resolution_clock::now();
  //Unpack Camera Info
  int nx = as<int>(camera_info["nx"]);
  int ny = as<int>(camera_info["ny"]);
  int ns = as<int>(camera_info["ns"]);
  Float fov = as<Float>(camera_info["fov"]);
  NumericVector lookfromvec = as<NumericVector>(camera_info["lookfrom"]);
  NumericVector lookatvec = as<NumericVector>(camera_info["lookat"]);
  Float aperture = as<Float>(camera_info["aperture"]);
  NumericVector camera_up = as<NumericVector>(camera_info["camera_up"]);
  Float shutteropen = as<Float>(camera_info["shutteropen"]);
  Float shutterclose = as<Float>(camera_info["shutterclose"]);
  Float focus_distance = as<Float>(camera_info["focal_distance"]);
  NumericVector ortho_dimensions = as<NumericVector>(camera_info["ortho_dimensions"]);
  size_t max_depth = as<size_t>(camera_info["max_depth"]);
  size_t roulette_active = as<size_t>(camera_info["roulette_active_depth"]);
  int sample_method = as<int>(camera_info["sample_method"]);
  NumericVector stratified_dim = as<NumericVector>(camera_info["stratified_dim"]);
  NumericVector light_direction = as<NumericVector>(camera_info["light_direction"]);
  int bvh_type = as<int>(camera_info["bvh"]);
  
  //Initialize output matrices
  NumericMatrix routput(nx,ny);
  NumericMatrix goutput(nx,ny);
  NumericMatrix boutput(nx,ny);
  
  vec3f lookfrom(lookfromvec[0],lookfromvec[1],lookfromvec[2]);
  vec3f lookat(lookatvec[0],lookatvec[1],lookatvec[2]);
  vec3f backgroundhigh(bghigh[0],bghigh[1],bghigh[2]);
  vec3f backgroundlow(bglow[0],bglow[1],bglow[2]);
  float dist_to_focus = focus_distance;
  CharacterVector alpha_files = as<CharacterVector>(alphalist["alpha_temp_file_names"]);
  LogicalVector has_alpha = as<LogicalVector>(alphalist["alpha_tex_bool"]);
  
  CharacterVector bump_files = as<CharacterVector>(alphalist["bump_temp_file_names"]);
  LogicalVector has_bump = as<LogicalVector>(alphalist["bump_tex_bool"]);
  NumericVector bump_intensity = as<NumericVector>(alphalist["bump_intensity"]);
  
  CharacterVector roughness_files = as<CharacterVector>(roughness_list["rough_temp_file_names"]);
  LogicalVector has_roughness = as<LogicalVector>(roughness_list["rough_tex_bool"]);
  
  RcppThread::ThreadPool pool(numbercores);
  GetRNGstate();
  random_gen rng(unif_rand() * std::pow(2,32));
  camera cam(lookfrom, lookat, vec3f(camera_up(0),camera_up(1),camera_up(2)), fov, float(nx)/float(ny), 
             aperture, dist_to_focus,
             shutteropen, shutterclose);
  
  ortho_camera ocam(lookfrom, lookat, vec3f(camera_up(0),camera_up(1),camera_up(2)),
                    ortho_dimensions(0), ortho_dimensions(1),
                    shutteropen, shutterclose);
  
  environment_camera ecam(lookfrom, lookat, vec3f(camera_up(0),camera_up(1),camera_up(2)),
                          shutteropen, shutterclose);
  int nx1, ny1, nn1;
  auto start = std::chrono::high_resolution_clock::now();
  if(verbose) {
    Rcpp::Rcout << "Building BVH: ";
  }
  
  std::vector<Float* > textures;
  std::vector<int* > nx_ny_nn;
  
  std::vector<Float* > alpha_textures;
  std::vector<int* > nx_ny_nn_alpha;
  
  std::vector<Float* > bump_textures;
  std::vector<int* > nx_ny_nn_bump;
  
  std::vector<Float* > roughness_textures;
  std::vector<int* > nx_ny_nn_roughness;
  //Shared material vector
  std::vector<std::shared_ptr<material> >* shared_materials = new std::vector<std::shared_ptr<material> >;
  
  for(int i = 0; i < n; i++) {
    if(isimage(i)) {
      int nx, ny, nn;
      Float* tex_data = stbi_loadf(filelocation(i), &nx, &ny, &nn, 0);
      textures.push_back(tex_data);
      nx_ny_nn.push_back(new int[3]);
      nx_ny_nn[i][0] = nx;
      nx_ny_nn[i][1] = ny;
      nx_ny_nn[i][2] = nn;
    } else {
      textures.push_back(nullptr);
      nx_ny_nn.push_back(nullptr);
    }
    if(has_alpha(i)) {
      stbi_ldr_to_hdr_gamma(1.0f);
      int nxa, nya, nna;
      Float* tex_data_alpha = stbi_loadf(alpha_files(i), &nxa, &nya, &nna, 0);
      alpha_textures.push_back(tex_data_alpha);
      nx_ny_nn_alpha.push_back(new int[3]);
      nx_ny_nn_alpha[i][0] = nxa;
      nx_ny_nn_alpha[i][1] = nya;
      nx_ny_nn_alpha[i][2] = nna;
      stbi_ldr_to_hdr_gamma(2.2f);
    } else {
      alpha_textures.push_back(nullptr);
      nx_ny_nn_alpha.push_back(nullptr);
    }
    if(has_bump(i)) {
      int nxb, nyb, nnb;
      Float* tex_data_bump = stbi_loadf(bump_files(i), &nxb, &nyb, &nnb, 0);
      bump_textures.push_back(tex_data_bump);
      nx_ny_nn_bump.push_back(new int[3]);
      nx_ny_nn_bump[i][0] = nxb;
      nx_ny_nn_bump[i][1] = nyb;
      nx_ny_nn_bump[i][2] = nnb;
    } else {
      bump_textures.push_back(nullptr);
      nx_ny_nn_bump.push_back(nullptr);
    }
    if(has_roughness(i)) {
      NumericVector temp_glossy = as<NumericVector>(glossyinfo(i));
      int nxr, nyr, nnr;
      Float* tex_data_roughness = stbi_loadf(roughness_files(i), &nxr, &nyr, &nnr, 0);
      Float min = temp_glossy(9), max = temp_glossy(10);
      Float rough_range = max-min;
      Float maxr = 0, minr = 1;
      for(int ii = 0; ii < nxr; ii++) {
        for(int jj = 0; jj < nyr; jj++) {
          Float temp_rough = tex_data_roughness[nnr*ii + nnr*nxr*jj];
          maxr = maxr < temp_rough ? temp_rough : maxr;
          minr = minr > temp_rough ? temp_rough : minr;
          if(nnr > 1) {
            temp_rough = tex_data_roughness[nnr*ii + nnr*nxr*jj+1];
            maxr = maxr < temp_rough ? temp_rough : maxr;
            minr = minr > temp_rough ? temp_rough : minr;
          }
        }
      }
      Float data_range = maxr-minr;
      for(int ii = 0; ii < nxr; ii++) {
        for(int jj = 0; jj < nyr; jj++) {
          if(!temp_glossy(11)) {
            tex_data_roughness[nnr*ii + nnr*nxr*jj] = 
              (tex_data_roughness[nnr*ii + nnr*nxr*jj]-minr)/data_range * rough_range + min;
            if(nnr > 1) {
              tex_data_roughness[nnr*ii + nnr*nxr*jj+1] = 
                (tex_data_roughness[nnr*ii + nnr*nxr*jj+1]-minr)/data_range * rough_range + min;
            }
          } else {
            tex_data_roughness[nnr*ii + nnr*nxr*jj] = 
              (1.0-(tex_data_roughness[nnr*ii + nnr*nxr*jj]-minr)/data_range) * rough_range + min;
            if(nnr > 1) {
              tex_data_roughness[nnr*ii + nnr*nxr*jj+1] = 
                (1.0-(tex_data_roughness[nnr*ii + nnr*nxr*jj+1]-minr)/data_range) * rough_range + min;
            }
          }
        }
      }
      roughness_textures.push_back(tex_data_roughness);
      nx_ny_nn_roughness.push_back(new int[3]);
      nx_ny_nn_roughness[i][0] = nxr;
      nx_ny_nn_roughness[i][1] = nyr;
      nx_ny_nn_roughness[i][2] = nnr;
    } else {
      roughness_textures.push_back(nullptr);
      nx_ny_nn_roughness.push_back(nullptr);
    }
  }
  
  //Initialize transformation cache
  TransformCache transformCache;
  
  
  std::shared_ptr<hitable> worldbvh = build_scene(type, radius, shape, position_list,
                                properties, 
                                n,shutteropen,shutterclose,
                                ischeckered, checkercolors, 
                                gradient_info,
                                noise, isnoise, noisephase, noiseintensity, noisecolorlist,
                                angle, 
                                isimage, has_alpha, alpha_textures, nx_ny_nn_alpha,
                                textures, nx_ny_nn, has_bump, bump_textures, nx_ny_nn_bump,
                                bump_intensity,
                                roughness_textures, nx_ny_nn_roughness, has_roughness,
                                lightintensity, isflipped,
                                isvolume, voldensity, order_rotation_list, 
                                isgrouped, group_transform,
                                tri_normal_bools, is_tri_color, tri_color_vert, 
                                fileinfo, filebasedir, 
                                scale_list, sigmavec, glossyinfo,
                                shared_id_mat, is_shared_mat, shared_materials,
                                image_repeat, csg_info, mesh_list, bvh_type, transformCache, 
                                animation_info, rng);
  auto finish = std::chrono::high_resolution_clock::now();
  if(verbose) {
    std::chrono::duration<double> elapsed = finish - start;
    Rcpp::Rcout << elapsed.count() << " seconds" << "\n";
  }
  
  //Calculate world bounds and ensure camera is inside infinite area light
  aabb bounding_box_world;
  worldbvh->bounding_box(0,0,bounding_box_world);
  Float world_radius = bounding_box_world.diag.length()/2 ;
  vec3f world_center  = bounding_box_world.centroid;
  world_radius = world_radius > (lookfrom - world_center).length() ? world_radius : (lookfrom - world_center).length()*2;
  
  if(fov == 0) {
    Float ortho_diag = sqrt(pow(ortho_dimensions(0),2) + pow(ortho_dimensions(1),2));
    world_radius += ortho_diag;
  }
  
  //Initialize background
  if(verbose && hasbackground) {
    Rcpp::Rcout << "Loading Environment Image: ";
  }
  start = std::chrono::high_resolution_clock::now();
  std::shared_ptr<texture> background_texture = nullptr;
  std::shared_ptr<material> background_material = nullptr;
  std::shared_ptr<hitable> background_sphere = nullptr;
  Float *background_texture_data = nullptr;
  
  //Background rotation
  Matrix4x4 Identity;
  Transform BackgroundAngle(Identity);
  if(rotate_env != 0) {
    BackgroundAngle = Translate(world_center) * RotateY(rotate_env);
  } 
  std::shared_ptr<Transform> BackgroundTransform = transformCache.Lookup(BackgroundAngle);
  std::shared_ptr<Transform> BackgroundTransformInv = transformCache.Lookup(BackgroundAngle.GetInverseMatrix());
  
  if(hasbackground) {
    background_texture_data = stbi_loadf(background[0], &nx1, &ny1, &nn1, 0);
    background_texture = std::make_shared<image_texture>(background_texture_data, nx1, ny1, nn1, 1, 1, intensity_env);
    background_material = std::make_shared<diffuse_light>(background_texture, 1.0, false);
    background_sphere = std::make_shared<InfiniteAreaLight>(nx1, ny1, world_radius*2, vec3f(0.f),
                                              background_texture, background_material, 
                                              BackgroundTransform,
                                              BackgroundTransformInv, false);
  } else if(ambient_light) {
    //Check if both high and low are black, and set to FLT_MIN
    if(backgroundhigh.length() == 0 && backgroundlow.length() == 0) {
      backgroundhigh = vec3f(FLT_MIN,FLT_MIN,FLT_MIN);
      backgroundlow = vec3f(FLT_MIN,FLT_MIN,FLT_MIN);
    }
    background_texture = std::make_shared<gradient_texture>(backgroundlow, backgroundhigh, false, false);
    background_material = std::make_shared<diffuse_light>(background_texture, 1.0, false);
    background_sphere = std::make_shared<InfiniteAreaLight>(100, 100, world_radius*2, vec3f(0.f),
                                              background_texture, background_material,
                                              BackgroundTransform,BackgroundTransformInv,false);
  } else {
    //Minimum intensity FLT_MIN so the CDF isn't NAN
    background_texture = std::make_shared<constant_texture>(vec3f(FLT_MIN,FLT_MIN,FLT_MIN));
    background_material = std::make_shared<diffuse_light>(background_texture, 1.0, false);
    background_sphere = std::make_shared<InfiniteAreaLight>(100, 100, world_radius*2, vec3f(0.f),
                                              background_texture, background_material,
                                              BackgroundTransform,
                                              BackgroundTransformInv,false);
  }
  finish = std::chrono::high_resolution_clock::now();
  if(verbose && hasbackground) {
    std::chrono::duration<double> elapsed = finish - start;
    Rcpp::Rcout << elapsed.count() << " seconds" << "\n";
  }
  int numbertosample = 0;
  for(int i = 0; i < implicit_sample.size(); i++) {
    if(implicit_sample(i)) {
      numbertosample++;
    }
  }
  hitable_list world;
  
  world.add(worldbvh);
  
  bool impl_only_bg = false;
  if(numbertosample == 0 || hasbackground || ambient_light) {
    world.add(background_sphere);
    impl_only_bg = true;
  }

  hitable_list hlist;
  if(verbose) {
    Rcpp::Rcout << "Building Importance Sampling List: ";
  }
  start = std::chrono::high_resolution_clock::now();
  for(int i = 0; i < n; i++)  {
    if(implicit_sample(i)) {
      hlist.add(build_imp_sample(type, radius, shape, position_list,
                               properties, 
                               n, shutteropen, shutterclose,
                               angle, i, order_rotation_list,
                               isgrouped, group_transform,
                               fileinfo, filebasedir,
                               transformCache ,scale_list, 
                               mesh_list,bvh_type, animation_info,  rng));
    }
  }
  finish = std::chrono::high_resolution_clock::now();
  if(verbose) {
    std::chrono::duration<double> elapsed = finish - start;
    Rcpp::Rcout << elapsed.count() << " seconds" << "\n";
  }
  if(impl_only_bg || hasbackground) {
    hlist.add(background_sphere);
  }

  if(verbose && !progress_bar) {
    Rcpp::Rcout << "Starting Raytracing:\n ";
  }
  RProgress::RProgress pb_sampler("Generating Samples [:bar] :percent%");
  pb_sampler.set_width(70);
  RProgress::RProgress pb("Adaptive Raytracing [:bar] :percent%");
  pb.set_width(70);

  if(progress_bar) {
    pb_sampler.set_total(ny);
    pb.set_total(ns);
  }
  if(min_variance == 0) {
    min_adaptive_size = 1;
    min_variance = 10E-8;
  }
  if(debug_channel != 0) {
    debug_scene(numbercores, nx, ny, ns, debug_channel,
                min_variance, min_adaptive_size, 
                routput, goutput,boutput,
                progress_bar, sample_method, stratified_dim,
                verbose, ocam, cam, ecam, fov,
                world, hlist,
                clampval, max_depth, roulette_active, 
                light_direction, rng);
  } else {
    pathtracer(numbercores, nx, ny, ns, debug_channel,
               min_variance, min_adaptive_size, 
               routput, goutput,boutput,
               progress_bar, sample_method, stratified_dim,
               verbose, ocam, cam, ecam, fov,
               world, hlist,
               clampval, max_depth, roulette_active);
  }

  if(verbose) {
    Rcpp::Rcout << "Cleaning up memory..." << "\n";
  }
  if(hasbackground) {
    stbi_image_free(background_texture_data);
  }
  for(int i = 0; i < n; i++) {
    if(isimage(i)) {
      stbi_image_free(textures[i]);
      delete nx_ny_nn[i];
    } 
    if(has_alpha(i)) {
      stbi_image_free(alpha_textures[i]);
      delete nx_ny_nn_alpha[i];
    }
    if(has_bump(i)) {
      stbi_image_free(bump_textures[i]);
      delete nx_ny_nn_bump[i];
    }
    if(has_roughness(i)) {
      stbi_image_free(roughness_textures[i]);
      delete nx_ny_nn_roughness[i];
    }
  }
  delete shared_materials;
  PutRNGstate();
  finish = std::chrono::high_resolution_clock::now();
  if(verbose) {
    std::chrono::duration<double> elapsed = finish - startfirst;
    Rcpp::Rcout << "Total time elapsed: " << elapsed.count() << " seconds" << "\n";
  }
  return(List::create(_["r"] = routput, _["g"] = goutput, _["b"] = boutput));
}
