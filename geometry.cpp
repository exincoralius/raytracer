#include "geometry.h"
#include <limits>
#include <math.h>
#include <stdio.h>
#include <iostream>

#define EPSILON 1e-4

/*
 * Class Point
 */
Point::Point() {
  x = 0;
  y = 0;
  z = 0;
}

Point::Point(float xVal, float yVal, float zVal) {
  x = xVal;
  y = yVal;
  z = zVal;
}

void Point::add(Vector v) {
  x+=v.x;
  y+=v.y;
  z+=v.z;
}

void Point::sub(Vector v) {
  x-=v.x;
  y-=v.y;
  z-=v.z;
}

Vector Point::sub(Point p) {
  return Vector(x-p.x, y-p.y, z-p.z);
}

/*
 * Class Vector
 */
Vector::Vector() {
  x = 0;
  y = 0;
  z = 0;
  // this->normalize();
}

Vector::Vector(float xVal, float yVal, float zVal) {
  x = xVal;
  y = yVal;
  z = zVal;
}

Vector::Vector(Point point) {
  x = point.x;
  y = point.y;
  z = point.z;
}

void Vector::scale(float k) {
  x = x * k;
  y = y * k;
  z = z * k;
}

void Vector::normalize() {
  float l2norm = norm();
  l2norm = 1.0 / l2norm;
  scale(l2norm);
}

float Vector::norm() {
  return sqrt(pow(x,2) + pow(y,2) + pow(z,2));
}

float Vector::dot(Vector vector) {
  return x * vector.x + y * vector.y + z * vector.z;
}

Vector Vector::cross(Vector v) {
  return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

void Vector::add(Vector vector) {
  x = x + vector.x;
  y = y + vector.y;
  z = z + vector.z;
}

void Vector::sub(Vector vector) {
  x = x - vector.x;
  y = y - vector.y;
  z = z - vector.z;
}

void Vector::add(Point point) {
  x = x + point.x;
  y = y + point.y;
  z = z + point.z;
}

void Vector::sub(Point point) {
  x = x - point.x;
  y = y - point.y;
  z = z - point.z;
}

/*
 * Class Ray
 */
Ray::Ray() {
  origin = Point();
  dir = Vector();
}

Ray::Ray(Point orig, Vector direction) {
  origin = orig;
  dir = direction;
  dir.normalize();
  tMin = EPSILON;
  tMax = std::numeric_limits<float>::infinity();
}

Ray::Ray(Point orig, Vector direction, float tMaxVal) {
  origin = orig;
  dir = direction;
  dir.normalize();
  tMin = EPSILON;
  tMax = tMaxVal;
}

Ray::Ray(Point orig, Vector direction, float tMinVal, float tMaxVal) {
  origin = orig;
  dir = direction;
  dir.normalize();
  tMin = tMinVal;
  tMax = tMaxVal;
}

Point Ray::at(float t) {
  if (t < tMin) t = tMin;
  else if (t > tMax) t = tMax;
  Point point = origin;
  Vector direction = dir;
  direction.scale(t);
  point.add(direction);
  return point;
}

LocalGeo::LocalGeo() {
  pos = Point();
  normal = Vector();
}

LocalGeo::LocalGeo(Point p, Vector n) {
  pos = p;
  normal = n;
  normal.normalize();
}

Point LocalGeo::getPosition() {
  return pos;
}

Vector LocalGeo::getNormal() {
  return normal;
}

void LocalGeo::setPosition(Point p) {
  pos = p;
}

void LocalGeo::setNormal(Vector n) {
  normal = n;
  normal.normalize();
}

float Shape::intersect(Ray& ray) {
}

float Shape::intersect(Ray& ray, LocalGeo* geo) {
}

Sphere::Sphere() {
  center = Point();
  radius = 1;
}

Sphere::Sphere(Point c, float r) {
  center = c;
  radius = r;
}

float Sphere::intersect(Ray& ray) {
  float a = ray.dir.dot(ray.dir);
  Vector temp = ray.origin;
  temp.sub(center);
  float b = ray.dir.dot(temp) * 2;
  float c = temp.dot(temp) - pow(radius,2);
  float discriminant = pow(b,2) - 4*a*c;

  if (discriminant < 0) return -1;
  else if (discriminant == 1) {
    float t = -b/(2*a);
    if (t >= ray.tMin && t <= ray.tMax) return t;
    else return -1.0;
  } else {
    float t1 = (-b + sqrt(discriminant)) / (2*a);
    float t2 = (-b - sqrt(discriminant)) / (2*a);
    // Rest of the code assumes that t1 <= t2
    if (t2 < t1) {
      double temp = t1;
      t1 = t2;
      t2 = temp;
    }
    if (t1 <= ray.tMax && t1 >= ray.tMin) return t1;
    else if (t2 >= ray.tMin && t2 <= ray.tMax) return t2;
    else return -1.0;
  }
}

float Sphere::intersect(Ray& ray, LocalGeo* local) {
  float intersection = intersect(ray);
  if (intersection != -1.0) {
    // std::cout << "RAY:" << std::endl;
    // std::cout << ray.origin.x << " " << ray.origin.y << " " << ray.origin.z << std::endl;
    // std::cout << ray.dir.x << " " << ray.dir.y << " " << ray.dir.z << std::endl;
    Point point = ray.at(intersection);
    Vector normal = Vector(point);
    local->setPosition(point);
    normal.sub(center);
    normal.normalize();
    // std::cout << "SHAPE NORMAL:" << std::endl;
    // std::cout << normal.x << " " << normal.y << " " << normal.z << std::endl;
    local->setNormal(normal);
  }
  return intersection;
}

Triangle::Triangle() {
  vertex = Point();
  edgeOne = Vector();
  edgeTwo = Vector();
}

Triangle::Triangle(Point point1, Point point2, Point point3) {
  vertex = point1;
  edgeOne = point2.sub(point1);
  edgeTwo = point3.sub(point1);
}

float Triangle::intersect(Ray& ray) {
  // Citation:
  // http://tinyurl.com/2zve8a
  Vector pVec = ray.dir.cross(edgeTwo);
  float det = pVec.dot(edgeOne);
  if (det > -EPSILON && det < EPSILON) return -1.0;
  Vector tVec = ray.origin;
  tVec.sub(vertex);
  float u, v;
  u = tVec.dot(pVec)/det;
  if (u>1.0 || u<0.0) return -1.0;
  Vector qVec = tVec.cross(edgeOne);
  v = ray.dir.dot(qVec)/det;
  if (v+u>1.0 || v<0.0) return -1.0;
  float t = edgeTwo.dot(qVec)/det;
  if (t<ray.tMin || t>ray.tMax) return -1.0;
  return t;
}

float Triangle::intersect(Ray& ray, LocalGeo* local) {
  float intersection = intersect(ray);
  if (intersection != -1.0) {
    local->setPosition(ray.at(intersection));
    Vector triNormal = Vector(edgeOne.cross(edgeTwo));
    triNormal.normalize();
    Vector geomNormal;
    geomNormal = Vector(edgeOne.cross(edgeTwo));
    geomNormal.normalize();
    local->setNormal(geomNormal);
  }
  return intersection;
}
