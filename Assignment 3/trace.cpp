#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include <cstdlib>

extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int reflections_on;
extern int chessboard_on;
extern int refractions_on;
extern int stochastic_on;
extern int supersampling_on;
extern int step_max;

extern Point chess_point;
extern Vector chess_board_normal;

extern float cb_ambient_black[3];    
extern float cb_ambient_white[3];    
extern float cb_diffuse_black[3];
extern float cb_diffuse_white[3];
extern float cb_specular_black[3];
extern float cb_specular_white[3];
extern float cb_shineness;
extern float cb_reflectance;

RGB_float phongChessboard(Point q, Vector v, Vector surf_norm, int base_col){
  RGB_float col, amb, spec, dif; //various constants for the equation
  col = {0, 0, 0}; amb = {0, 0, 0}; spec = {0, 0, 0}; dif = {0, 0, 0}; //initialization
  normalize(&surf_norm);

  float dist_q_light; //distance between light source and point on object
  float D_n_dot_L; //diffuse parameter
  float NH_n_dot_H;//specular shininess parameter
  float decay; //coefficient for light decay (specular and diffuse only)

  Vector H; //half-way vector between v and L (for specular)
  Vector L; //vec from point q to lightsource

  float ambient[3];
  float diffuse[3];
  float specular[3];

  if(base_col==0){
    for(int i = 0; i<3; i++){
      ambient[i] = cb_ambient_black[i];
      diffuse[i] = cb_diffuse_black[i];
      specular[i] = cb_specular_black[i];  
    }
  }  
  else{
    for(int i = 0; i<3; i++){
      ambient[i] = cb_ambient_white[i];
      diffuse[i] = cb_diffuse_white[i];
      specular[i] = cb_specular_white[i];  
    }
  }

  //AMBIENT GLOBAL
  amb.r = amb.r + ambient[0]*2*global_ambient[0]*cb_reflectance;
  amb.g = amb.g + ambient[1]*2*global_ambient[1]*cb_reflectance;
  amb.b = amb.b + ambient[2]*2*global_ambient[2]*cb_reflectance;

  Vector Shadow = get_vec(q, light1);
  if((shadow_on) && (isLightBlocked(q, Shadow, scene))){ //needs to be fixed for chessboard, not a sphere!
    return amb;
  }

  L = get_vec(q, light1); 
  dist_q_light = vec_len(L);
  normalize(&L); //prior to doing casting operations

  //light decay as defined in assignment specs
  decay = (1.0/
          ( decay_a + 
            decay_b * dist_q_light + 
            decay_c*pow(dist_q_light, 2) 
          ));

  //DIFFUSE FROM DIRECT HITS < 90*
  D_n_dot_L = vec_dot(surf_norm, L);
  dif.r = dif.r + light1_diffuse[0]*diffuse[0]*D_n_dot_L;
  dif.g = dif.g + light1_diffuse[1]*diffuse[1]*D_n_dot_L;
  dif.b = dif.b + light1_diffuse[2]*diffuse[2]*D_n_dot_L;

  //SPECULAR FROM DIRECT HITS < 90*
  //reflectance vector = ray - 2*(ray(dot)normal)*normal
  float ref_scalar = 2.0*(vec_dot(surf_norm, L));
  H = vec_plus(vec_scale(surf_norm, ref_scalar), vec_scale(L, -1.0));
  NH_n_dot_H = pow(vec_dot(v, H), cb_shineness);
  spec.r = spec.r + light1_specular[0]*specular[0]*NH_n_dot_H;
  spec.g = spec.g + light1_specular[1]*specular[1]*NH_n_dot_H;
  spec.b = spec.b + light1_specular[2]*specular[2]*NH_n_dot_H;

  //add it all up
  col = clr_add(dif, spec);
  col = clr_scale(col, decay);
  col = clr_add(col, amb);

  return col;
}

RGB_float boardColor(Point hit, Vector u){ //assuming we already have a hitpoint, we return 1 of 2 colors
  int mod_op = 2;
  RGB_float ret_color;
  if( (int(hit.x) % mod_op == 0 && int(hit.z) % mod_op == 0) || (int(hit.x) % mod_op != 0 && int(hit.z) % mod_op != 0)){ 
    if(hit.x <= 0) ret_color = phongChessboard(hit, u, chess_board_normal, 0); //{0,0,0};
    else ret_color = phongChessboard(hit, u, chess_board_normal, 1); //{1, 1, 1};
  }
  else{
    if(hit.x <= 0) ret_color = phongChessboard(hit, u, chess_board_normal, 1); //{1,1,1};
    else ret_color = phongChessboard(hit, u, chess_board_normal, 0); //{0, 0, 0};
  } 
  return ret_color;
}

bool intersect_chessboard(Vector u, Point o, Point *hit){
  normalize(&chess_board_normal);
  Vector origin_to_plane_point = get_vec(chess_point, o);
  float normal_dot_o_to_p = vec_dot(chess_board_normal, origin_to_plane_point);
  float normal_dot_ray = vec_dot(chess_board_normal, u);
  if(normal_dot_ray == 0) return false; //paralell to board
  if(normal_dot_o_to_p == 0) return false;

  float r = normal_dot_o_to_p / normal_dot_ray;

  if(r <= 0) return false;
  else{
    u = vec_scale(u, r);
    o = get_point(o, u);
    hit->x = o.x;
    hit->y = o.y;
    hit->z = o.z;
    if(o.x >= 4.0 || o.x < -4.0) return false;
    if(o.z >= -2 || o.z < -10) return false;
    return true;
  }
}

RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres *sph) {

  RGB_float col, amb, spec, dif; //various constants for the equation
  col = {0, 0, 0}; amb = {0, 0, 0}; spec = {0, 0, 0}; dif = {0, 0, 0}; //initialization

  float dist_q_light; //distance between light source and point on object
  float D_n_dot_L; //diffuse parameter
  float NH_n_dot_H;//specular shininess parameter
  float A_sph_ref; //reflectance parameter
  float decay; //coefficient for light decay (specular and diffuse only)

  Vector H; //half-way vector between v and L (for specular)
  Vector L; //vec from point q to lightsource

  //AMBIENT GLOBAL
  A_sph_ref = sph->reflectance;
  amb.r = amb.r + global_ambient[0]*A_sph_ref;
  amb.g = amb.g + global_ambient[1]*A_sph_ref;
  amb.b = amb.b + global_ambient[2]*A_sph_ref;

  Vector Shadow = get_vec(q, light1);
  if((shadow_on) && (isLightBlocked(q, Shadow, scene))){
    return amb;
  }

  L = get_vec(q, light1); 
  dist_q_light = vec_len(L);
  normalize(&L); //prior to doing casting operations

  //light decay as defined in assignment specs
  decay = (1.0/
          ( decay_a + 
            decay_b * dist_q_light + 
            decay_c*pow(dist_q_light, 2) 
          ));

  //DIFFUSE FROM DIRECT HITS < 90*
  D_n_dot_L = vec_dot(surf_norm, L);
  dif.r = dif.r + light1_diffuse[0]*sph->mat_diffuse[0]*D_n_dot_L;
  dif.g = dif.g + light1_diffuse[1]*sph->mat_diffuse[1]*D_n_dot_L;
  dif.b = dif.b + light1_diffuse[2]*sph->mat_diffuse[2]*D_n_dot_L;

  //SPECULAR FROM DIRECT HITS < 90*
  //reflectance vector = ray - 2*(ray(dot)normal)*normal
  H = vec_plus(L, vec_scale(surf_norm, -2*vec_dot(L, surf_norm)));
  NH_n_dot_H = pow(vec_dot(v, H), sph->mat_shineness);
  spec.r = spec.r + light1_specular[0]*sph->mat_specular[0]*NH_n_dot_H;
  spec.g = spec.g + light1_specular[1]*sph->mat_specular[1]*NH_n_dot_H;
  spec.b = spec.b + light1_specular[2]*sph->mat_specular[2]*NH_n_dot_H;

  //add it all up
  col = clr_add(dif, spec);
  col = clr_scale(col, decay);
  col = clr_add(col, clr_scale(amb, 2));

  return col;
}

Vector transmissionVector(Vector i, Vector surf_norm, Spheres *nearest, Point *hit){
  
  Vector transmission;

  Point temp;
  temp.x = hit->x;
  temp.y = hit->y;
  temp.z = hit->z;
  Vector eye_to_hit = get_vec(eye_pos, temp);
  Vector eye_to_center = get_vec(eye_pos, nearest->center);
  float length_eye_to_hit = vec_len(eye_to_hit);
  normalize(&eye_to_hit);
  float length_projection = vec_dot(eye_to_center, eye_to_hit);

  float n;

  n = 1.0/(nearest->refraction_index);

  Vector transmission_parallel = vec_scale(vec_minus(i, vec_scale(surf_norm, vec_dot(i, surf_norm))), n);

  if(vec_len(transmission_parallel) > 1.0){
    transmission = {0, 0, 0};
    return transmission;
  }

  Vector transmission_orthogonal = vec_scale(surf_norm, (-1)*(sqrt(1-pow(vec_dot(transmission_parallel, transmission_parallel), 2))));    
  transmission = vec_plus(transmission_parallel, transmission_orthogonal);

  return transmission;
}

RGB_float recursive_ray_trace(Vector ray, Point o, int iteration) {
  
  RGB_float ret_color, reflection_col, refraction_col;

  ret_color = background_clr; 
  reflection_col = {0, 0, 0}; 
  refraction_col = {0, 0, 0}; //not yet used

  Spheres *nearest; //initialization
  Point *hit = new Point;

  nearest = intersect_scene(o, ray, scene, hit);
      
  if(nearest != NULL){

    Vector view = get_vec(*hit, eye_pos);
    normalize(&view);
    Vector surf_norm = sphere_normal(*hit, nearest);
    normalize(&surf_norm);
    Vector L = get_vec(*hit, o);
    normalize(&L);

    ret_color = phong(*hit, view, surf_norm, nearest);
    
    if((reflections_on) && (iteration < step_max)){
    //check for reflectance intersections for up to step_max 
    //reflection rays
      float ref_scalar = 2.0*(vec_dot(surf_norm, L));
      Vector reflection = vec_plus(vec_scale(surf_norm, ref_scalar), vec_scale(L, -1.0));
      normalize(&reflection);
      iteration+=1;

      reflection_col = recursive_ray_trace(reflection, *hit, iteration);
      reflection_col = clr_scale(reflection_col, nearest->reflectance); //how much color is this reflection keeping
      ret_color = clr_add(ret_color, reflection_col);
    }

    if ((refractions_on) && (iteration < step_max))
    {
      Vector transmission = transmissionVector(view, surf_norm, nearest, hit);
      //Vector transmission = getRefractedRay(1.51, nearest, surf_norm, L);
      normalize(&transmission);
      iteration+=1;
      transmission.x = hit->x + transmission.x;//transmission.x; 
      transmission.x = hit->y + transmission.y;//transmission.y;
      transmission.x = hit->z + transmission.z;//transmission.z;
      refraction_col = recursive_ray_trace(transmission, *hit, iteration);
      refraction_col = clr_scale(refraction_col, nearest->transparency);
      ret_color = clr_add(ret_color, refraction_col);
    }
    return ret_color;
  }

  if(chessboard_on){    
    Point *chessboard_hit = new Point;  
    bool isBoardHit = intersect_chessboard(ray, o, chessboard_hit);
    if(isBoardHit){
      Vector view = get_vec(*chessboard_hit, eye_pos);
      normalize(&view);
      Vector L = get_vec(*chessboard_hit, o);
      normalize(&L);
      ret_color = boardColor(*chessboard_hit, view);
      normalize(&chess_board_normal);
      
      if((reflections_on) && (iteration < step_max)){

        float ref_scalar = 2.0*(vec_dot(chess_board_normal, L));
        Vector reflection = vec_plus(vec_scale(chess_board_normal, ref_scalar), vec_scale(L, -1.0));
        normalize(&reflection);
        
        iteration+=1;

        reflection_col = recursive_ray_trace(reflection, *chessboard_hit, iteration);
        reflection_col = clr_scale(reflection_col, cb_reflectance); //how much color is this reflection keeping
        ret_color = clr_add(ret_color, reflection_col);
      }
      return ret_color;
    }
  }
  return ret_color;
}

void ray_trace() {
  int i, j;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  if(supersampling_on){
    printf("IN SUPER SAMPLER\n");
    Point sub_pix_pos;
    RGB_float cumulative_color;
    
    for (i=0; i<win_height; i++) {
      for (j=0; j<win_width; j++) {
        sub_pix_pos = cur_pixel_pos;

        ray = get_vec(eye_pos, sub_pix_pos);
        normalize(&ray);      
        ret_color = recursive_ray_trace(ray, eye_pos, 0);
        cumulative_color = clr_add(ret_color, cumulative_color);

        sub_pix_pos.x -= 0.25 * x_grid_size;
        sub_pix_pos.y -= 0.25 * y_grid_size;
        ray = get_vec(eye_pos, sub_pix_pos);
        normalize(&ray);      
        ret_color = recursive_ray_trace(ray, eye_pos, 0);
        cumulative_color = clr_add(ret_color, cumulative_color);

        sub_pix_pos.x += 0.25 * x_grid_size;
        sub_pix_pos.y -= 0.25 * y_grid_size;
        ray = get_vec(eye_pos, sub_pix_pos);
        normalize(&ray);      
        ret_color = recursive_ray_trace(ray, eye_pos, 0);
        cumulative_color = clr_add(ret_color, cumulative_color);

        sub_pix_pos.x -= 0.25 * x_grid_size;
        sub_pix_pos.y += 0.25 * y_grid_size;
        ray = get_vec(eye_pos, sub_pix_pos);
        normalize(&ray);      
        ret_color = recursive_ray_trace(ray, eye_pos, 0);
        cumulative_color = clr_add(ret_color, cumulative_color);

        sub_pix_pos.x += 0.25 * x_grid_size;
        sub_pix_pos.y += 0.25 * y_grid_size;
        ray = get_vec(eye_pos, sub_pix_pos);
        normalize(&ray);      
        ret_color = recursive_ray_trace(ray, eye_pos, 0);
        cumulative_color = clr_add(ret_color, cumulative_color);

        frame[i][j][0] = GLfloat(ret_color.r);
        frame[i][j][1] = GLfloat(ret_color.g);
        frame[i][j][2] = GLfloat(ret_color.b);

        cur_pixel_pos.x += x_grid_size;
      }

      cur_pixel_pos.y += y_grid_size;
      cur_pixel_pos.x = x_start;
    }
  }
  else{ 
    for (i=0; i<win_height; i++) {
      for (j=0; j<win_width; j++) {      
        ray = get_vec(eye_pos, cur_pixel_pos);
        normalize(&ray);      
        ret_color = recursive_ray_trace(ray, eye_pos, 0);
        frame[i][j][0] = GLfloat(ret_color.r);
        frame[i][j][1] = GLfloat(ret_color.g);
        frame[i][j][2] = GLfloat(ret_color.b);

        cur_pixel_pos.x += x_grid_size;
      }

      cur_pixel_pos.y += y_grid_size;
      cur_pixel_pos.x = x_start;
    }
  }

  if(stochastic_on){
    printf("IN STOCHASTIC CASTER\n");
        for(int i = 0; i < 5; i++){
          int r1, r2, n1, n2;
          r1 = rand() % (int)image_width/2;
          r2 = rand() % (int)image_height/2;
          n1 = rand() % 2;
          n2 = rand() % 2;

          if(n1 == 0) r1 = -r1;
          if(n2 == 0) r2 = -r2;

          cur_pixel_pos.x = (float) r1;
          cur_pixel_pos.y = (float) r2;
          cur_pixel_pos.z = image_plane;

          ray = get_vec(eye_pos, cur_pixel_pos);
          normalize(&ray);
          ret_color = recursive_ray_trace(ray, eye_pos, 0);

          frame[r1 + win_height/2][r2+win_width/2][0] = GLfloat(ret_color.r);
          frame[r1 + win_height/2][r2+win_width/2][1] = GLfloat(ret_color.g);
          frame[r1 + win_height/2][r2+win_width/2][2] = GLfloat(ret_color.b);
        }
      }  
}
