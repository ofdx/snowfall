/*
 * WedgeTerrain
 * mperron (2021)
 * 
 * A system for generating grid-based terrain, using cubes which can be
 * sub-divided into four wedge-shaped pieces.
 * 
 */
#include <bitset>

#define WEDGE_BYTE_COUNT   8
#define LAYER_EDGE_LENGTH 16

class WedgeTerrain : public Scene3D::Renderable {
    list<Scene3D::Mesh*> active_meshes;

    struct Wedge {
        uint8_t flags, alpha;
        uint16_t geometry, color_top_a, color_top_b, color_bottom_a, color_bottom_b;

        Wedge(){
            flags = 0;
            geometry = 0x7ff;
            alpha = 0x1f; // (5-bit unsigned max)

            // 10-bit reference into the color palette for this terrain map.
            color_top_a = 1;
            color_top_b = 2;
            color_bottom_a = 3;
            color_bottom_b = 4;
        }

        Wedge(uint8_t *datap){
            flags    = datap[0];
            geometry = datap[1] | (((datap[2] >> 5) & 0x7) << 8);
            alpha    = datap[2] & 0x1f;

            color_top_a    = datap[3] |  ((datap[7]       & 0x3) << 8);
            color_top_b    = datap[4] | (((datap[7] >> 2) & 0x3) << 8);
            color_bottom_a = datap[5] | (((datap[7] >> 4) & 0x3) << 8);
            color_bottom_b = datap[6] | (((datap[7] >> 6) & 0x3) << 8);
        }

        void repack(uint8_t *packed){
            packed[0] = flags;
            packed[1] = geometry;
            packed[2] = alpha | ((geometry >> 3) & 0xe0);

            packed[3] = color_top_a;
            packed[4] = color_top_b;
            packed[5] = color_bottom_a;
            packed[6] = color_bottom_b;
            packed[7] =
                (((color_top_a    >> 8) & 0x3)       |
                (((color_top_b    >> 8) & 0x3) << 2) |
                (((color_bottom_a >> 8) & 0x3) << 4) |
                (((color_bottom_b >> 8) & 0x3) << 6));
        }

        void display(){
            bitset<8> b_flags(flags);
            bitset<11> b_geometry(geometry);
            bitset<5> b_alpha(alpha);
            bitset<10> b_color_top_a(color_top_a);
            bitset<10> b_color_top_b(color_top_b);
            bitset<10> b_color_bottom_a(color_bottom_a);
            bitset<10> b_color_bottom_b(color_bottom_b);

            cout << "flags   |geometry   |alpha|top a     |top b     |bottom a  |bottom b  |" << endl;
            cout
                << b_flags << "|"
                << b_geometry << "|" << b_alpha << "|"
                << b_color_top_a << "|" << b_color_top_b << "|"
                << b_color_bottom_a << "|" << b_color_bottom_b << "|"
                << endl;
        }
    };

    class Sector {
    public:
        struct Header {
            int16_t offset_x;
            int16_t offset_z;
            int8_t  offset_layer;
            uint8_t layer_count;
        } header;

        vector<uint8_t*> *layers;

        Sector(int16_t offsetX, int16_t offsetZ, int8_t offsetLayer, uint8_t layerCount){
            header = {
                offset_x:     offsetX,
                offset_z:     offsetZ,
                offset_layer: offsetLayer,
                layer_count:  layerCount
            };

            layers = new vector<uint8_t*>(layerCount);

            for(int i = 0; i < layerCount; i++){
                (*layers)[i] = (uint8_t*) calloc(WEDGE_BYTE_COUNT * LAYER_EDGE_LENGTH * LAYER_EDGE_LENGTH, sizeof(uint8_t));
            }
        }
        ~Sector(){
            for(uint8_t ui = 0; ui < header.layer_count; ui++)
                free((*layers)[ui]);
                
            delete layers;
        }

        void populateLayer(uint8_t layerIndex){
            // Fill the layer with full cubes, each with a different side color.
            uint8_t *p = (*layers)[layerIndex];
            Wedge w;

            w.alpha = 0x1f; // Max alpha
            w.color_top_a = 0;
            w.color_top_b = 1;
            w.color_bottom_a = 2;
            w.color_bottom_b = 3;
            w.geometry = 0b11001010101;

            // Generate byte data.
            uint8_t packed[8];
            w.repack(packed);

            for(int i = 0; i < LAYER_EDGE_LENGTH * LAYER_EDGE_LENGTH; i++)
                memcpy(p + (i * WEDGE_BYTE_COUNT), packed, WEDGE_BYTE_COUNT);
        }

        unordered_map<int, int> coord_cache;
        int vertForCoord(vector<Scene3D::coord> &vertices, const Scene3D::coord &c){
            int index = c.x + (c.z * (1 + LAYER_EDGE_LENGTH)) + (c.y * (1 + LAYER_EDGE_LENGTH) * (1 + LAYER_EDGE_LENGTH));

            if(vertices.size() != 0){
                int vertId = coord_cache[index];
                Scene3D::coord d = vertices[vertId];

                if(c == d){
                    // Found a match!
                    return vertId;
                } else {
                    vertId = vertices.size();

                    // False positive. Probably vertId == 0 (not found).
                    vertices.push_back(c);
                    return (coord_cache[index] = vertId);
                }
            } else {
                // First entry in the list; insert the coord and return 0.
                vertices.push_back(c);
                return (coord_cache[index] = 0);
            }
        }

        Scene3D::Mesh *calculateMesh(Scene3D::Camera *cam, unsigned long &count_layers, unsigned long &count_verts, unsigned long &count_faces){
            list<Scene3D::Mesh::Face*> faces;
            vector<Scene3D::coord> vertices;

            for(int layerIndex = 0; layerIndex < header.layer_count; layerIndex++){
                for(int i = 0; i < LAYER_EDGE_LENGTH * LAYER_EDGE_LENGTH; i++){
                    // Offset position into this layer.
                    double x = (header.offset_x * LAYER_EDGE_LENGTH) + (i % LAYER_EDGE_LENGTH);
                    double z = (header.offset_z * LAYER_EDGE_LENGTH) + (i / LAYER_EDGE_LENGTH);
                    double y = (header.offset_layer + layerIndex);

                    // FIXME - reference a palette instead.
                    const byte_t fill_a[4] = { 0xff, 0x7f, 0x7f, 0xff };
                    const byte_t fill_b[4] = { 0x7f, 0xff, 0x7f, 0xff };
                    const byte_t fill_c[4] = { 0x7f, 0x7f, 0xff, 0xff };
                    const byte_t fill_d[4] = { 0xff, 0xff, 0x7f, 0xff };

                    // Add faces to mesh.
                    {
                        Scene3D::coord c = { .x = x, .y = y, .z = z };

                        // Modifiers to reach the other coords.
                        Scene3D::coord
                            cxp = { .x =  1 },
                            cyp = { .y =  1 },
                            czp = { .z =  1 };

                        uint8_t *p = (*layers)[layerIndex];
                        Wedge w(p);

                        #define WEDGE_VERT_0 vertIds.push_back(vertForCoord(vertices, c))
                        #define WEDGE_VERT_1 vertIds.push_back(vertForCoord(vertices, c + cxp))
                        #define WEDGE_VERT_2 vertIds.push_back(vertForCoord(vertices, c + cxp + czp))
                        #define WEDGE_VERT_3 vertIds.push_back(vertForCoord(vertices, c + czp))
                        #define WEDGE_VERT_4 vertIds.push_back(vertForCoord(vertices, c + cyp))
                        #define WEDGE_VERT_5 vertIds.push_back(vertForCoord(vertices, c + cyp + cxp))
                        #define WEDGE_VERT_6 vertIds.push_back(vertForCoord(vertices, c + cyp + cxp + czp))
                        #define WEDGE_VERT_7 vertIds.push_back(vertForCoord(vertices, c + cyp + czp))

                        // FIXME - use geometry from Wedge.

                        // Bottom A
                        {

                            vector<int> vertIds;
                            WEDGE_VERT_0; WEDGE_VERT_1; WEDGE_VERT_2;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_0; WEDGE_VERT_1; WEDGE_VERT_5;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_1; WEDGE_VERT_2; WEDGE_VERT_5;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_2; WEDGE_VERT_0; WEDGE_VERT_5;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
                        }

                        // Bottom B
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_2; WEDGE_VERT_3; WEDGE_VERT_0;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_2; WEDGE_VERT_3; WEDGE_VERT_7;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_3; WEDGE_VERT_0; WEDGE_VERT_7;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_0; WEDGE_VERT_2; WEDGE_VERT_7;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
                        }

                        // Top A
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_5; WEDGE_VERT_6; WEDGE_VERT_7;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_5; WEDGE_VERT_6; WEDGE_VERT_2;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_6; WEDGE_VERT_7; WEDGE_VERT_2;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_7; WEDGE_VERT_5; WEDGE_VERT_2;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
                        }

                        // Top B
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_7; WEDGE_VERT_4; WEDGE_VERT_5;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_7; WEDGE_VERT_4; WEDGE_VERT_0;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_4; WEDGE_VERT_5; WEDGE_VERT_0;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
                        }
                        {
                            vector<int> vertIds;
                            WEDGE_VERT_5; WEDGE_VERT_7; WEDGE_VERT_0;
                            faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
                        }
                    }
                }
            }

            // FIXME debug
            cout << "calculateMesh [layers=" << (unsigned int) header.layer_count << "] [verts=" << vertices.size() << "] [faces=" << faces.size() << "]" << endl;

            count_layers += header.layer_count;
            count_verts += vertices.size();
            count_faces += faces.size();

            // Clear this cache. It's not useful once the mesh is generated.
            coord_cache.clear();

            return new Scene3D::Mesh(cam, vertices, faces);
        }
    };

    list<Sector*> sectors;

    void calculateMesh(){
        active_meshes.clear();

        unsigned long count_layers = 0, count_verts = 0, count_faces = 0;

        for(Sector *sector : sectors)
            active_meshes.push_back(sector->calculateMesh(cam, count_layers, count_verts, count_faces));

        // FIXME debug
        cout << "calculateMesh TOTAL [layers=" << count_layers << "] [verts=" << count_verts << "] [faces=" << count_faces << "]" << endl;
    }

public:
    WedgeTerrain(Scene3D::Camera *cam)
        : Renderable(cam)
    {
        // FIXME debug - A single test sector, one layer tall, in the middle of the playfield.
        sectors.push_back(new Sector(0, 0, 0, 2));
        sectors.push_back(new Sector(1, 0, 1, 1));
        sectors.push_back(new Sector(0,-1, 0, 1));

        // Called when terrain changes.
        calculateMesh();
    }

    ~WedgeTerrain(){
        for(Scene3D::Mesh *mesh : active_meshes)
            delete mesh;

        for(Sector *sector : sectors)
            delete sector;
    }

    virtual void draw(int ticks){
        for(Scene3D::Mesh *mesh : active_meshes)
            mesh->draw(ticks);
    }
};