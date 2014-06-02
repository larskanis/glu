require 'mkmf'

def have_framework(fw, &b)
  checking_for fw do
    src = cpp_include("#{fw}/#{fw}.h") << "\n" "int main(void){return 0;}"
    if try_link(src, opt = "-ObjC -framework #{fw}", &b)
      $defs.push(format("-DHAVE_FRAMEWORK_%s", fw.tr_cpp))
      $LDFLAGS << " " << opt
      true
    else
      false
    end
  end
end unless respond_to? :have_framework

if ENV['CROSS_COMPILING']
  dir_config("installed")
end

ok =
  (have_library('opengl32.lib', 'glVertex3d') &&
   have_library('glu32.lib',    'gluLookAt')) ||
  (have_library('opengl32') &&
   have_library('glu32')) ||
  (have_library('GL',   'glVertex3d') &&
   have_library('GLU',  'gluLookAt')) ||
  (have_framework('OpenGL') && have_framework('GLUT') && have_framework('Cocoa'))

ok &&=
  (have_header('GL/gl.h') && have_header('GL/glu.h')) ||
  (have_header('OpenGL/gl.h') && have_header('OpenGL/glu.h')) # OS X

have_struct_member 'struct RFloat', 'float_value'

if ok then
  create_header
  create_makefile 'glu/glu'
end
