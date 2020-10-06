#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <cfloat>


float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit) {
	//point o = origin (eye point)
  //vector u = unit ray vector from origin
  //first we check whether the sphere is 'behind' the ray
  Vector proj_cntr_on_u;
  float dist_test;

  Point cntr = sph->center;
  float rad = sph->radius;
  Vector eye_to_cntr = get_vec(o, cntr);
  float dp_ray_cntr = vec_dot(u, eye_to_cntr);
  float dist_to_cntr = vec_len(eye_to_cntr);

    
    if(dp_ray_cntr <= 0) return -1.0; //center of sphere does not project onto ray 

    proj_cntr_on_u = vec_scale(u, dp_ray_cntr); //ONLY WORKS IF u HAS BEEN NORMALIZED

    dist_test = vec_len(vec_minus(proj_cntr_on_u, eye_to_cntr)); //distance from center to nearest point on ray

    if(dist_test > rad) return -1.0;
    else if(dist_test == rad){
      hit->x = proj_cntr_on_u.x;
      hit->y = proj_cntr_on_u.y;
      hit->z = proj_cntr_on_u.z;
      return dp_ray_cntr; //scalar for projection, in this case
    } 
    else{ //2 intersections, return closest
      float param = sqrt(rad*rad - dist_test*dist_test);
      param = vec_len(proj_cntr_on_u) - param; //parameter for scaling IF u IS NORMALIZED
      Point inter_1 = get_point(o, vec_scale(u, param));
      hit->x = inter_1.x;
      hit->y = inter_1.y;
      hit->z = inter_1.z;
      //printf("Parameter: %f\n", param);
      return param; 
    }
  
  return 0.0; // should never reach this point
}

bool isLightBlocked(Point o, Vector u, Spheres *sph_iter)
{
  
  while(sph_iter != NULL)
  {
    //use quadratic equation to find intersect
      Point cntr = sph_iter->center;
      float rad = sph_iter->radius;
      float a = vec_dot(u,u); 
      Vector cntr_to_origin = get_vec(cntr, o);
      float b = vec_dot(cntr_to_origin, vec_scale(u, 2));        
      Vector temp_cntr = {cntr.x, cntr.y, cntr.z};
      Vector temp_o = {o.x, o.y, o.z};
      float c = vec_dot(temp_cntr, temp_cntr) + vec_dot(temp_o, temp_o) - vec_dot(temp_cntr, temp_o)*2 - pow(rad, 2);
      float disc = pow(b,2) - 4*a*c;
      float disc_sqrt = sqrt(disc);
      float r1 = (-b - disc_sqrt)/2;
      float r2 = (-b + disc_sqrt)/2;

      if(disc > 0){
        if(r1 > 0.0001 || r2 > 0.0001)
        return true;
      }    
    sph_iter = sph_iter->next;
  }
  return false;
}

Spheres *intersect_scene(Point o, Vector u, Spheres *sph, Point *hit) {
// taking input ray, iterate over sphere objects, calling intersect 
// sphere at each object

  Spheres *sph_iter = sph;
  Spheres *nearest = NULL;

  float nearest_dist, current_dist; 
  nearest_dist = FLT_MAX;


  while(sph_iter != NULL) {
    current_dist = intersect_sphere(o, u, sph_iter, hit);
    if((nearest_dist > current_dist) && (current_dist != -1.0)) {
      nearest_dist = current_dist; 
      nearest = sph_iter;
    }    
    sph_iter = sph_iter->next;    
  }
  return nearest;
}

Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, float refract, float alpha, int sindex) {
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->refraction_index = refract;
  new_sphere->transparency = alpha;
  new_sphere->next = NULL;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}
