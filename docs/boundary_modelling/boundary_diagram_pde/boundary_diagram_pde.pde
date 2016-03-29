import processing.pdf.*;

void setup() {
  size(500, 500, P3D);
  colorMode(RGB, 1);
  noLoop();
}

void line(PVector a, PVector b) {line(a.x, a.y, a.z, b.x, b.y, b.z);}
void translate(PVector p) {translate(p.x, p.y, p.z);}

void sphereAt(PVector p) {
  pushMatrix();
  translate(p);
  sphere(5);
  popMatrix();
}

void lineFrom(PVector p, PVector direction, float l) {
  pushMatrix();
  translate(p);
  line(new PVector(0, 0, 0), PVector.mult(direction, l));
  popMatrix();
}

void draw() {
  beginRaw(PDF, "boundary_diagram.pdf");
  
  background(1);
  
  noFill();
  strokeWeight(1);
  stroke(0);
  
  translate(width / 2, height / 2);
  rotateX(-PI / 6);
  rotateY(3 * PI / 4);
    
  int grid_lines = 5;
  float l = width * 0.2;
  
  for (int i = 0; i != grid_lines - 1; ++i) {
    float lerped = lerp(-l, l, (i + 0) / (grid_lines - 1.0));
    
    line(new PVector(-l, lerped, -l), new PVector(l, lerped, -l));
    line(new PVector(lerped, -l, -l), new PVector(lerped, l, -l));
    
    line(new PVector(-l, -l, lerped), new PVector(-l, l, lerped));
    line(new PVector(-l, lerped, -l), new PVector(-l, lerped, l));
    
    line(new PVector(-l, -l, lerped), new PVector(l, -l, lerped));
    line(new PVector(lerped, -l, -l), new PVector(lerped, -l, l));
  }
  
  float connector_length = 1 * l / grid_lines;
  
  strokeWeight(1);
  stroke(1, 0, 0);
  fill(1, 0, 0);
  sphereAt(new PVector(-l, -l, -l));
  strokeWeight(5);
  lineFrom(new PVector(-l, -l, -l), new PVector(1, 0, 0), connector_length);
  lineFrom(new PVector(-l, -l, -l), new PVector(0, 1, 0), connector_length);
  lineFrom(new PVector(-l, -l, -l), new PVector(0, 0, 1), connector_length);

  for (int i = 0; i != grid_lines - 2; ++i) {
    float lerped = lerp(-l, l, (i + 1) / (grid_lines - 1.0));
    stroke(0, 1, 0);
    fill(0, 1, 0);
    
    strokeWeight(1);
    sphereAt(new PVector(-l, -l, lerped));
    strokeWeight(5);
    lineFrom(new PVector(-l, -l, lerped), new PVector(1, 0, 0), connector_length);
    lineFrom(new PVector(-l, -l, lerped), new PVector(0, 1, 0), connector_length);
    
    strokeWeight(1);
    sphereAt(new PVector(-l, lerped, -l));
    strokeWeight(5);
    lineFrom(new PVector(-l, lerped, -l), new PVector(1, 0, 0), connector_length);
    lineFrom(new PVector(-l, lerped, -l), new PVector(0, 0, 1), connector_length);
    
    strokeWeight(1);
    sphereAt(new PVector(lerped, -l, -l));
    strokeWeight(5);
    lineFrom(new PVector(lerped, -l, -l), new PVector(0, 1, 0), connector_length);
    lineFrom(new PVector(lerped, -l, -l), new PVector(0, 0, 1), connector_length);
    
    for (int j = 0; j != grid_lines - 2; ++j) {
      float l2 = lerp(-l, l, (j + 1) / (grid_lines - 1.0));
      
      stroke(0, 0, 1);
      fill(0, 0, 1);
      
      strokeWeight(1);
      sphereAt(new PVector(lerped, l2, -l));
      strokeWeight(5);
      lineFrom(new PVector(lerped, l2, -l), new PVector(0, 0, 1), connector_length);
      
      strokeWeight(1);
      sphereAt(new PVector(l2, -l, lerped));
      strokeWeight(5);
      lineFrom(new PVector(l2, -l, lerped), new PVector(0, 1, 0), connector_length);
      
      strokeWeight(1);
      sphereAt(new PVector(-l, lerped, l2));
      strokeWeight(5);
      lineFrom(new PVector(-l, lerped, l2), new PVector(1, 0, 0), connector_length);
    }
  }
  endRaw();
}