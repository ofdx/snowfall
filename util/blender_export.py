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
                # Name for this object
                if obdata.id_data != None:
                    pfs(fs, '{} '.format(obdata.id_data.name))

                pfs(fs, '{\n')

                # Vertices
                pfs(fs, '\tcoords {\n')
                for v in obdata.vertices:
                    # Apply the object's translations
                    co = obj.matrix_world * v.co
                    pfs(fs, '\t\t{{ {:0.3f} {:0.3f} {:0.3f} }} # Vert {:3d}\n'.format(co.x, co.z, co.y, v.index))

                pfs(fs, '\t}\n')


                # Find fill color from material
                try:
                    color = obj.active_material.diffuse_color

                    # RGBA
                    fill = '{:02x}{:02x}{:02x}ff'.format(
                        (int)(color[0] * 0xff),
                        (int)(color[1] * 0xff),
                        (int)(color[2] * 0xff)
                    )
                except Exception as e:
                    # A nice default blue color.
                    fill = "336699ff"

                # Faces
                pfs(fs, '\tfaces {\n')
                for f in obdata.polygons:
                    pfs(fs, '\t\t{ ')

                    for v in f.vertices:
                        pfs(fs, '{:3d} '.format(v))

                    pfs(fs, '}} {} # Face {:3d}\n'.format(fill, f.index))
                pfs(fs, '\t}\n');
                pfs(fs, '}\n\n')
        except:
            pass

    # Mark the end of the file.
    pfs(fs, '\nEOF\n');
