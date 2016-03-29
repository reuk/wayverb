import processing.pdf.*;

void setup() {
  size(500, 500, P3D);
  colorMode(RGB, 1);
  noLoop();
}

void line(PVector a, PVector b) {line(a.x, a.y, a.z, b.x, b.y, b.z);}
void translate(PVector p) {translate(p.x, p.y, p.z);}

void sphereAt(PVector p) {
  line(new PVector(0, 0, 0), p);
  pushMatrix();
  translate(p);
  sphere(5);
  popMatrix();
}

PVector s(float cosz, float sinz, float theta) {
    return new PVector(cosz * cos(theta), sinz, cosz * sin(theta));
}

PVector point_on_sphere(float az, float el) {
    return s(cos(el), sin(el), az);
}

void draw() {
  beginRaw(PDF, "testing_setup.pdf");
  
  background(1);
  
  noFill();
  strokeWeight(1);
  stroke(0);
  
  translate(width / 2, height / 2);
  rotateX(-PI / 6);
  rotateY(3.5 * PI / 4);
  
  float azimuth = PI / 6;
  float elevation = PI / 6;
  
  float l = width * 0.3;
  pushMatrix();
  translate(-l / 2, 0, 0);
  box(l);
  popMatrix();
  
  pushMatrix();
  translate(l / 2, 0, 0);
  box(l);
  popMatrix();
  
  float dist = sqrt(l * l + l * l + l * l) / 4;
  
  PVector source = PVector.mult(point_on_sphere(azimuth + PI, elevation), dist);
  
  fill(1, 0, 0);
  stroke(1, 0, 0);
  sphereAt(source);
  fill(0, 1, 0);
  stroke(0, 1, 0);
  sphereAt(PVector.mult(source, -1));
  fill(0, 0, 1);
  stroke(0, 0, 1);
  sphereAt(new PVector(source.x, -source.y, -source.z));
    
  endRaw();
}