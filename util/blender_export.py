# Export mesh to text which can be used by my picogamo.

import bpy
import os

def pfs(fs, str):
    fs.write(str)
    print(str, end='')

with open(os.path.splitext(bpy.data.filepath)[0] + '.txt', 'w') as fs:
    for obj in bpy.data.objects:
        obdata = obj.data

        try:
            if obdata != None and obdata.vertices != None and obdata.polygons != None:
                # Friendly mesh name
                if obdata.id_data != None:
                    pfs(fs, '// Mesh: {}\n'.format(obdata.id_data.name))

                pfs(fs, '{\n')
                pfs(fs, '\tvector<Scene3D::coord> coords {\n')
                for v in obdata.vertices:
                    # Apply the object's translations
                    co = obj.matrix_world * v.co

                    pfs(fs, '\t\t{{ {:0.3f}, {:0.3f}, {:0.3f} }}, // Vert {:03d}\n'.format(co.x, co.z, co.y, v.index))

                pfs(fs, '\t};\n')

                pfs(fs, '\tlist<vector<int>> faces {\n')
                for f in obdata.polygons:
                    pfs(fs, '\t\t{ ')

                    for v in f.vertices:
                        pfs(fs, '{:3d}, '.format(v))

                    pfs(fs, '}}, // Face {:03d}\n'.format(f.index))
                pfs(fs, '\t};\n');
                pfs(fs, '\tScene3D::Mesh *obj = new Scene3D::Mesh(cam, coords, faces);\n')
                pfs(fs, '\n')
                pfs(fs, '\trendered_meshes.push_back(obj);\n')
                pfs(fs, '\tdrawable_meshes.push_back(obj);\n')
                pfs(fs, '}\n\n')
        except:
            pass
