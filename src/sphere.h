#ifndef SPHEREH
#define SPHEREH

#include "hitable.h"
#include "material.h"
#include "mathinline.h"
#include "transform.h"
#include "matrix.h"

class sphere: public hitable {
  public:
    sphere() {}
    ~sphere() {}
    sphere(vec3f cen, Float r, std::shared_ptr<material> mat, 
           std::shared_ptr<alpha_texture> alpha_mask, std::shared_ptr<bump_texture> bump_tex,
           std::shared_ptr<Transform> ObjectToWorld, std::shared_ptr<Transform> WorldToObject, bool reverseOrientation) : 
            hitable(ObjectToWorld, WorldToObject, reverseOrientation), 
            center(cen), radius(r), 
            mat_ptr(mat), alpha_mask(alpha_mask), bump_tex(bump_tex) {};
    virtual bool hit(const ray& r, Float tmin, Float tmax, hit_record& rec, random_gen& rng);
    virtual bool hit(const ray& r, Float tmin, Float tmax, hit_record& rec, Sampler* sampler);
    
    virtual bool bounding_box(Float t0, Float t1, aabb& box) const;
    virtual Float pdf_value(const point3f& o, const vec3f& v, random_gen& rng, Float time = 0);
    virtual Float pdf_value(const point3f& o, const vec3f& v, Sampler* sampler, Float time = 0);
    
    virtual vec3f random(const point3f& o, random_gen& rng, Float time = 0);
    virtual vec3f random(const point3f& o, Sampler* sampler, Float time = 0);
    
    point3f center;
    Float radius;
    std::shared_ptr<material> mat_ptr;
    std::shared_ptr<alpha_texture> alpha_mask;
    std::shared_ptr<bump_texture> bump_tex;
};

class moving_sphere: public hitable {
  public:
    moving_sphere() {}
    moving_sphere(vec3f cen0, vec3f cen1, Float t0, Float t1, Float r, 
                  std::shared_ptr<material> mat,
                  std::shared_ptr<alpha_texture> alpha_mask, std::shared_ptr<bump_texture> bump_tex,
                  std::shared_ptr<Transform> ObjectToWorld, std::shared_ptr<Transform> WorldToObject, bool reverseOrientation) : 
                    hitable(ObjectToWorld, WorldToObject, reverseOrientation), 
                    center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), 
                    mat_ptr(mat), alpha_mask(alpha_mask), bump_tex(bump_tex) {};
    virtual bool hit(const ray& r, Float tmin, Float tmax, hit_record& rec, random_gen& rng);
    virtual bool hit(const ray& r, Float tmin, Float tmax, hit_record& rec, Sampler* sampler);
    
    virtual bool bounding_box(Float t0, Float t1, aabb& box) const;
    virtual Float pdf_value(const point3f& o, const vec3f& v, random_gen& rng, Float time = 0);
    virtual Float pdf_value(const point3f& o, const vec3f& v, Sampler* sampler, Float time = 0);
    
    virtual vec3f random(const point3f& o, random_gen& rng, Float time = 0);
    virtual vec3f random(const point3f& o, Sampler* sampler, Float time = 0);
    point3f center(Float time) const;
    point3f center0, center1;
    Float time0, time1;
    Float radius;
    std::shared_ptr<material> mat_ptr;
    std::shared_ptr<alpha_texture> alpha_mask;
    std::shared_ptr<bump_texture> bump_tex;
};

#endif
