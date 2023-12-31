/*
 * WedgeTerrain
 * mperron (2021)
 *
 * A system for generating grid-based terrain, using cubes which can be
 * sub-divided into four wedge-shaped pieces.
 *
 */

/*
8 byte/64 bits:
	bitset<9> b_flags(flags);
	bitset<10> b_geometry(geometry);
	bitset<5> b_alpha(alpha);
	bitset<10> b_color_top_a(color_top_a);
	bitset<10> b_color_top_b(color_top_b);
	bitset<10> b_color_bottom_a(color_bottom_a);
	bitset<10> b_color_bottom_b(color_bottom_b);

geometry
nn f aa bb cc dd

n not used
n not used
f flip 90 degrees
a top a
a top a
b top b
b top b
c bottom a
c bottom a
d bottom b
d bottom b

two bit wedge modes:
00 - no display
01 - normal wedge outside edges
10 - completer wedge (complement slope)
11 - inside face only (internal diamond)

*/

#include <bitset>

#define WEDGE_BYTE_COUNT   8
#define LAYER_EDGE_LENGTH 4
#define LAYER_RENDER_HEIGHT 1.0
#define LAYER_RENDER_WIDTH  1.5

class WedgeTerrain : public Scene3D::Renderable {
	struct Wedge {
		enum WedgePieceMode {
			WEDGE_PIECE_MODE_NO_DISPLAY        = 0b00,
			WEDGE_PIECE_MODE_NORMAL            = 0b01,
			WEDGE_PIECE_MODE_COMPLEMENT_SLOPE  = 0b10,
			WEDGE_PIECE_MODE_INSIDE_ONLY       = 0b11
		};

		enum WedgePiece {
			WEDGE_PIECE_TOP_A,
			WEDGE_PIECE_TOP_B,
			WEDGE_PIECE_BOTTOM_A,
			WEDGE_PIECE_BOTTOM_B
		};

		uint8_t flags, alpha;
		uint16_t geometry, color_top_a, color_top_b, color_bottom_a, color_bottom_b;

		Wedge();
		Wedge(uint8_t const *datap);

		void unpack(uint8_t const *datap);
		void repack(uint8_t *packed) const;
		void display() const;

		WedgePieceMode getPieceMode(WedgePiece const& piece) const;
		bool is_flipped() const;

		bool has_full_face_xp() const;
		bool has_full_face_xn() const;
		bool has_full_face_zp() const;
		bool has_full_face_zn() const;
		bool has_full_face_yp() const;
		bool has_full_face_yn() const;
	};

	class Sector {
		unordered_map<int, int> coord_cache;
		uint64_t m_verts, m_faces;

	public:
		struct Header {
			int16_t offset_x;
			int16_t offset_z;
			int8_t  offset_layer;
			uint8_t layer_count;
		} header;

		vector<uint8_t*> *layers;

		Sector(int16_t const offsetX, int16_t const offsetZ, int8_t const offsetLayer, uint8_t const layerCount);
		~Sector();

		void populateLayer(uint8_t const layerIndex);

		int vertForCoord(vector<Scene3D::coord> &vertices, Scene3D::coord const& c);
		Scene3D::Mesh *calculateMesh(Scene3D::MultiThreadCamera *mtcam);

		// Debug utilities
		void reportGeometryTotals(uint64_t& layers, uint64_t& verts, uint64_t& faces) const;
	};

	Scene3D::MultiThreadCamera *mtcam;
	list<Scene3D::Mesh*> active_meshes;
	list<Sector*> sectors;

	void calculateMesh();

public:
	WedgeTerrain(Scene3D::MultiThreadCamera *cam);
	~WedgeTerrain();

	// Should call draw_if_cam instead.
	void draw(int ticks) override {}

	void draw_if_cam(int ticks, Scene3D::Camera const *refCam) override;
};


WedgeTerrain::Wedge::Wedge(){
	flags = 0;
	geometry = 0x7ff;
	alpha = 0x1f; // (5-bit unsigned max)

	// 10-bit reference into the color palette for this terrain map.
	color_top_a = 1;
	color_top_b = 2;
	color_bottom_a = 3;
	color_bottom_b = 4;
}
WedgeTerrain::Wedge::Wedge(uint8_t const *datap){
	unpack(datap);
}
void WedgeTerrain::Wedge::unpack(uint8_t const *datap){
	flags    = datap[0];
	geometry = datap[1] | (((datap[2] >> 5) & 0x7) << 8);
	alpha    = datap[2] & 0x1f;

	color_top_a    = datap[3] |  ((datap[7]       & 0x3) << 8);
	color_top_b    = datap[4] | (((datap[7] >> 2) & 0x3) << 8);
	color_bottom_a = datap[5] | (((datap[7] >> 4) & 0x3) << 8);
	color_bottom_b = datap[6] | (((datap[7] >> 6) & 0x3) << 8);
}
void WedgeTerrain::Wedge::repack(uint8_t *packed) const {
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
void WedgeTerrain::Wedge::display() const {
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
WedgeTerrain::Wedge::WedgePieceMode WedgeTerrain::Wedge::getPieceMode(WedgeTerrain::Wedge::WedgePiece const& piece) const {
	WedgePieceMode mode = WEDGE_PIECE_MODE_NO_DISPLAY;

	switch(piece){
		case WEDGE_PIECE_TOP_A:
			mode = (WedgePieceMode)((geometry >> 6) & 0b11);
			break;
		case WEDGE_PIECE_TOP_B:
			mode = (WedgePieceMode)((geometry >> 4) & 0b11);
			break;
		case WEDGE_PIECE_BOTTOM_A:
			mode = (WedgePieceMode)((geometry >> 2) & 0b11);
			break;
		case WEDGE_PIECE_BOTTOM_B:
			mode = (WedgePieceMode)(geometry & 0b11);
			break;
	}

	return mode;
}
bool WedgeTerrain::Wedge::is_flipped() const {
	return (geometry >> 8) & 1;
}
bool WedgeTerrain::Wedge::has_full_face_xp() const {
	// FIXME - consider flipped faces
	return (
		(getPieceMode(WEDGE_PIECE_BOTTOM_A) == WEDGE_PIECE_MODE_NORMAL) &&
		(is_flipped() ?
			(getPieceMode(WEDGE_PIECE_TOP_B) == WEDGE_PIECE_MODE_NORMAL) :
			(getPieceMode(WEDGE_PIECE_TOP_A) == WEDGE_PIECE_MODE_NORMAL)
		)
	);
}
bool WedgeTerrain::Wedge::has_full_face_xn() const {
	return (
		(getPieceMode(WEDGE_PIECE_BOTTOM_B) == WEDGE_PIECE_MODE_NORMAL) &&
		(is_flipped() ?
			(getPieceMode(WEDGE_PIECE_TOP_A) == WEDGE_PIECE_MODE_NORMAL) :
			(getPieceMode(WEDGE_PIECE_TOP_B) == WEDGE_PIECE_MODE_NORMAL)
		)
	);
}
bool WedgeTerrain::Wedge::has_full_face_zp() const {
	return (
		(is_flipped() ?
			(getPieceMode(WEDGE_PIECE_BOTTOM_A) == WEDGE_PIECE_MODE_NORMAL) :
			(getPieceMode(WEDGE_PIECE_BOTTOM_B) == WEDGE_PIECE_MODE_NORMAL)
		) &&
		(getPieceMode(WEDGE_PIECE_TOP_A) == WEDGE_PIECE_MODE_NORMAL)
	);
}
bool WedgeTerrain::Wedge::has_full_face_zn() const {
	return (
		(is_flipped() ?
			(getPieceMode(WEDGE_PIECE_BOTTOM_B) == WEDGE_PIECE_MODE_NORMAL) :
			(getPieceMode(WEDGE_PIECE_BOTTOM_A) == WEDGE_PIECE_MODE_NORMAL)
		) &&
		(getPieceMode(WEDGE_PIECE_TOP_B) == WEDGE_PIECE_MODE_NORMAL)
	);
}
bool WedgeTerrain::Wedge::has_full_face_yp() const {
	return (
		(getPieceMode(WEDGE_PIECE_TOP_A) == WEDGE_PIECE_MODE_NORMAL) &&
		(getPieceMode(WEDGE_PIECE_TOP_B) == WEDGE_PIECE_MODE_NORMAL)
	);
}
bool WedgeTerrain::Wedge::has_full_face_yn() const {
	return (
		(getPieceMode(WEDGE_PIECE_BOTTOM_A) == WEDGE_PIECE_MODE_NORMAL) &&
		(getPieceMode(WEDGE_PIECE_BOTTOM_B) == WEDGE_PIECE_MODE_NORMAL)
	);
}


WedgeTerrain::Sector::Sector(int16_t const offsetX, int16_t const offsetZ, int8_t const offsetLayer, uint8_t const layerCount) :
	m_verts(0), m_faces(0),
	header((WedgeTerrain::Sector::Header){
		offset_x:     offsetX,
		offset_z:     offsetZ,
		offset_layer: offsetLayer,
		layer_count:  layerCount
	}),
	layers(new vector<uint8_t*>(layerCount))
{
	for(int i = 0; i < layerCount; i++){
		(*layers)[i] = (uint8_t*) calloc(WEDGE_BYTE_COUNT * LAYER_EDGE_LENGTH * LAYER_EDGE_LENGTH, sizeof(uint8_t));

		// Default layer settings.
		populateLayer(i);
	}
}
WedgeTerrain::Sector::~Sector(){
	for(uint8_t ui = 0; ui < header.layer_count; ui++)
		free((*layers)[ui]);

	delete layers;
}
void WedgeTerrain::Sector::populateLayer(uint8_t const layerIndex){
	// Fill the layer with full cubes, each with a different side color.
	uint8_t *p = (*layers)[layerIndex];
	Wedge w;

	w.alpha = 0x1f; // Max alpha
	w.color_top_a = 0;
	w.color_top_b = 1;
	w.color_bottom_a = 2;
	w.color_bottom_b = 3;
	w.geometry = 0b001010101;

	// Generate byte data.
	uint8_t packed[8];
	w.repack(packed);

	for(int i = 0; i < LAYER_EDGE_LENGTH * LAYER_EDGE_LENGTH; i++)
		memcpy(p + (i * WEDGE_BYTE_COUNT), packed, WEDGE_BYTE_COUNT);
}
int WedgeTerrain::Sector::vertForCoord(vector<Scene3D::coord> &vertices, Scene3D::coord const& c){
	int const index = c.x + (c.z * (1 + LAYER_EDGE_LENGTH)) + (c.y * (1 + LAYER_EDGE_LENGTH) * (1 + LAYER_EDGE_LENGTH));

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
Scene3D::Mesh *WedgeTerrain::Sector::calculateMesh(Scene3D::MultiThreadCamera *mtcam){
	list<Scene3D::Mesh::Face*> faces;
	vector<Scene3D::coord> vertices;

	for(int layerIndex = 0; layerIndex < header.layer_count; layerIndex++){
		for(int i = 0; i < LAYER_EDGE_LENGTH * LAYER_EDGE_LENGTH; i++){
			// Offset position into this layer.
			double const x = LAYER_RENDER_WIDTH * ((header.offset_x * LAYER_EDGE_LENGTH) + (i % LAYER_EDGE_LENGTH));
			double const z = LAYER_RENDER_WIDTH * ((header.offset_z * LAYER_EDGE_LENGTH) + (i / LAYER_EDGE_LENGTH));
			double const y = LAYER_RENDER_HEIGHT * (header.offset_layer + layerIndex);

			// FIXME - reference a palette instead.
			byte_t const fill_a[4] = { 0xff, 0x7f, 0x7f, 0xff };
			byte_t const fill_b[4] = { 0x7f, 0xff, 0x7f, 0xff };
			byte_t const fill_c[4] = { 0x7f, 0x7f, 0xff, 0xff };
			byte_t const fill_d[4] = { 0xff, 0xff, 0x7f, 0xff };

			// Add faces to mesh.
			{
				Scene3D::coord const c = { .x = x, .y = y, .z = z };

				// Modifiers to reach the other coords.
				Scene3D::coord const
					cxp = { .x =  LAYER_RENDER_WIDTH },
					cyp = { .y =  LAYER_RENDER_HEIGHT },
					czp = { .z =  LAYER_RENDER_WIDTH };

				uint8_t *p = (*layers)[layerIndex] + i * WEDGE_BYTE_COUNT;
				Wedge w(p);

				// Adjacent wedges
				Wedge
					*w_xp = nullptr,
					*w_xn = nullptr,
					*w_zp = nullptr,
					*w_zn = nullptr,
					*w_yp = nullptr,
					*w_yn = nullptr;

				// TODO - get wedges from adjacent sectors
				if((i % LAYER_EDGE_LENGTH) < LAYER_EDGE_LENGTH - 1)
					w_xp = new Wedge((*layers)[layerIndex] + (i + 1) * WEDGE_BYTE_COUNT);
				if((i % LAYER_EDGE_LENGTH) > 0)
					w_xn = new Wedge((*layers)[layerIndex] + (i - 1) * WEDGE_BYTE_COUNT);
				if((i / LAYER_EDGE_LENGTH) < LAYER_EDGE_LENGTH - 1)
					w_zp = new Wedge((*layers)[layerIndex] + (i + LAYER_EDGE_LENGTH) * WEDGE_BYTE_COUNT);
				if((i / LAYER_EDGE_LENGTH) > 0)
					w_zn = new Wedge((*layers)[layerIndex] + (i - LAYER_EDGE_LENGTH) * WEDGE_BYTE_COUNT);

				if(layerIndex < header.layer_count - 1)
					w_yp = new Wedge((*layers)[layerIndex + 1] + i * WEDGE_BYTE_COUNT);
				if(layerIndex != 0)
					w_yn = new Wedge((*layers)[layerIndex - 1] + i * WEDGE_BYTE_COUNT);


				#define WEDGE_VERT_0 vertIds.push_back(vertForCoord(vertices, c))
				#define WEDGE_VERT_1 vertIds.push_back(vertForCoord(vertices, c + cxp))
				#define WEDGE_VERT_2 vertIds.push_back(vertForCoord(vertices, c + cxp + czp))
				#define WEDGE_VERT_3 vertIds.push_back(vertForCoord(vertices, c + czp))
				#define WEDGE_VERT_4 vertIds.push_back(vertForCoord(vertices, c + cyp))
				#define WEDGE_VERT_5 vertIds.push_back(vertForCoord(vertices, c + cyp + cxp))
				#define WEDGE_VERT_6 vertIds.push_back(vertForCoord(vertices, c + cyp + cxp + czp))
				#define WEDGE_VERT_7 vertIds.push_back(vertForCoord(vertices, c + cyp + czp))

				// FIXME - use geometry from Wedge.

				/*
				The cube model

				Lower(y) Upper(y+1)
				x +      x +
				z 0 1    z 4 5
				+ 3 2    + 7 6
				*/

				Wedge::WedgePieceMode
					wpm_top_a =    w.getPieceMode(Wedge::WedgePiece::WEDGE_PIECE_TOP_A),
					wpm_top_b =    w.getPieceMode(Wedge::WedgePiece::WEDGE_PIECE_TOP_B),
					wpm_bottom_a = w.getPieceMode(Wedge::WedgePiece::WEDGE_PIECE_BOTTOM_A),
					wpm_bottom_b = w.getPieceMode(Wedge::WedgePiece::WEDGE_PIECE_BOTTOM_B);

				bool is_incomplete_cube = !(
					(wpm_top_a == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NORMAL) &&
					(wpm_top_b == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NORMAL) &&
					(wpm_bottom_a == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NORMAL) &&
					(wpm_bottom_b == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NORMAL)
				);

				// Bottom A
				if(wpm_bottom_a != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NO_DISPLAY){
					if(wpm_bottom_a != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_INSIDE_ONLY){
						if(w.is_flipped()){
							if(!w_yn || !w_yn->has_full_face_yp()){
								vector<int> vertIds;
								WEDGE_VERT_1; WEDGE_VERT_2; WEDGE_VERT_3;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
							}
							if(!w_xp || !w_xp->has_full_face_xn()){
								vector<int> vertIds;
								WEDGE_VERT_1; WEDGE_VERT_2; WEDGE_VERT_6;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
							}
							if(!w_zp || !w_zp->has_full_face_zn()){
								vector<int> vertIds;
								WEDGE_VERT_2; WEDGE_VERT_3; WEDGE_VERT_6;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
							}
						} else {
							if(!w_yn || !w_yn->has_full_face_yp()){
								vector<int> vertIds;
								WEDGE_VERT_0; WEDGE_VERT_1; WEDGE_VERT_2;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
							}
							if(!w_zn || !w_zn->has_full_face_zp()){
								vector<int> vertIds;
								WEDGE_VERT_0; WEDGE_VERT_1; WEDGE_VERT_5;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
							}
							if(!w_xp || !w_xp->has_full_face_xn()){
								vector<int> vertIds;
								WEDGE_VERT_1; WEDGE_VERT_2; WEDGE_VERT_5;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
							}
						}
					}

					if(is_incomplete_cube){
						vector<int> vertIds;

						if(w.is_flipped()){
							WEDGE_VERT_1; WEDGE_VERT_3;

							if(wpm_bottom_a == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_4;
							} else {
								WEDGE_VERT_6;
							}
						} else {
							WEDGE_VERT_2; WEDGE_VERT_0;

							if(wpm_bottom_a == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_7;
							} else {
								WEDGE_VERT_5;
							}
						}

						faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_a));
					}
				}

				// Bottom B
				if(wpm_bottom_b != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NO_DISPLAY){
					if(wpm_bottom_b != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_INSIDE_ONLY){
						if(w.is_flipped()){
							if(!w_yn || !w_yn->has_full_face_yp()){
								vector<int> vertIds;
								WEDGE_VERT_3; WEDGE_VERT_0; WEDGE_VERT_1;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
							}
							if(!w_xn || !w_xn->has_full_face_xp()){
								vector<int> vertIds;
								WEDGE_VERT_3; WEDGE_VERT_0; WEDGE_VERT_4;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
							}
							if(!w_zn || !w_zn->has_full_face_zp()){
								vector<int> vertIds;
								WEDGE_VERT_0; WEDGE_VERT_1; WEDGE_VERT_4;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
							}
						} else {
							if(!w_yn || !w_yn->has_full_face_yp()){
								vector<int> vertIds;
								WEDGE_VERT_2; WEDGE_VERT_3; WEDGE_VERT_0;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
							}
							if(!w_zp || !w_zp->has_full_face_zn()){
								vector<int> vertIds;
								WEDGE_VERT_2; WEDGE_VERT_3; WEDGE_VERT_7;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
							}
							if(!w_xn || !w_xn->has_full_face_xp()){
								vector<int> vertIds;
								WEDGE_VERT_3; WEDGE_VERT_0; WEDGE_VERT_7;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
							}
						}
					}

					if(is_incomplete_cube){
						vector<int> vertIds;

						if(w.is_flipped()){
							WEDGE_VERT_1; WEDGE_VERT_3;

							if(wpm_bottom_b == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_6;
							} else {
								WEDGE_VERT_4;
							}
						} else {
							WEDGE_VERT_0; WEDGE_VERT_2;

							if(wpm_bottom_b == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_5;
							} else {
								WEDGE_VERT_7;
							}
						}

						faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_b));
					}
				}

				// Top A
				if(wpm_top_a != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NO_DISPLAY){
					if(wpm_top_a != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_INSIDE_ONLY){
						if(w.is_flipped()){
							if(!w_yp || !w_yp->has_full_face_yn()){
								vector<int> vertIds;
								WEDGE_VERT_6; WEDGE_VERT_7; WEDGE_VERT_4;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
							}
							if(!w_zp || !w_zp->has_full_face_zn()){
								vector<int> vertIds;
								WEDGE_VERT_6; WEDGE_VERT_7; WEDGE_VERT_3;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
							}
							if(!w_xn || !w_xn->has_full_face_xp()){
								vector<int> vertIds;
								WEDGE_VERT_7; WEDGE_VERT_0; WEDGE_VERT_3;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
							}
						} else {
							if(!w_yp || !w_yp->has_full_face_yn()){
								vector<int> vertIds;
								WEDGE_VERT_5; WEDGE_VERT_6; WEDGE_VERT_7;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
							}
							if(!w_xp || !w_xp->has_full_face_xn()){
								vector<int> vertIds;
								WEDGE_VERT_5; WEDGE_VERT_6; WEDGE_VERT_2;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
							}
							if(!w_zp || !w_zp->has_full_face_zn()){
								vector<int> vertIds;
								WEDGE_VERT_6; WEDGE_VERT_7; WEDGE_VERT_2;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
							}
						}
					}

					if(is_incomplete_cube){
						vector<int> vertIds;

						if(w.is_flipped()){
							WEDGE_VERT_4; WEDGE_VERT_6;

							if(wpm_top_a == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_1;
							} else {
								WEDGE_VERT_3;
							}
						} else {
							WEDGE_VERT_7; WEDGE_VERT_5;

							if(wpm_top_a == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_0;
							} else {
								WEDGE_VERT_2;
							}
						}

						faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_c));
					}
				}

				// Top B
				if(wpm_top_b != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_NO_DISPLAY){
					if(w.is_flipped()){
						if(wpm_top_b != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_INSIDE_ONLY){
							if(!w_yp || !w_yp->has_full_face_yn()){
								vector<int> vertIds;
								WEDGE_VERT_4; WEDGE_VERT_5; WEDGE_VERT_6;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
							}
							if(!w_zn || !w_zn->has_full_face_zp()){
								vector<int> vertIds;
								WEDGE_VERT_4; WEDGE_VERT_5; WEDGE_VERT_1;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
							}
							if(!w_xp || !w_xp->has_full_face_xn()){
								vector<int> vertIds;
								WEDGE_VERT_5; WEDGE_VERT_6; WEDGE_VERT_1;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
							}
						}
					} else {
						if(wpm_top_b != Wedge::WedgePieceMode::WEDGE_PIECE_MODE_INSIDE_ONLY){
							if(!w_yp || !w_yp->has_full_face_yn()){
								vector<int> vertIds;
								WEDGE_VERT_7; WEDGE_VERT_4; WEDGE_VERT_5;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
							}
							if(!w_xn || !w_xn->has_full_face_xp()){
								vector<int> vertIds;
								WEDGE_VERT_7; WEDGE_VERT_4; WEDGE_VERT_0;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
							}
							if(!w_zn || !w_zn->has_full_face_zp()){
								vector<int> vertIds;
								WEDGE_VERT_4; WEDGE_VERT_5; WEDGE_VERT_0;
								faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
							}
						}
					}

					if(is_incomplete_cube){
						vector<int> vertIds;

						if(w.is_flipped()){
							WEDGE_VERT_6; WEDGE_VERT_4;

							if(wpm_top_b == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_3;
							} else {
								WEDGE_VERT_1;
							}
						} else {
							WEDGE_VERT_5; WEDGE_VERT_7;

							if(wpm_top_b == Wedge::WedgePieceMode::WEDGE_PIECE_MODE_COMPLEMENT_SLOPE){
								WEDGE_VERT_2;
							} else {
								WEDGE_VERT_0;
							}
						}

						faces.push_back(new Scene3D::Mesh::Face(vertIds, fill_d));
					}
				}

				delete w_xp;
				delete w_xn;
				delete w_zp;
				delete w_zn;
				delete w_yp;
				delete w_yn;
			}
		}
	}

	// Clear this cache. It's not useful once the mesh is generated.
	coord_cache.clear();

	// Debug counters.
	m_verts = vertices.size();
	m_faces = faces.size();

	return new Scene3D::Mesh(mtcam->nextThreadCam(), vertices, faces);
}
void WedgeTerrain::Sector::reportGeometryTotals(uint64_t& layers, uint64_t& verts, uint64_t& faces) const {
	layers += header.layer_count;
	verts += m_verts;
	faces += m_faces;
}


void WedgeTerrain::calculateMesh(){
	for(Scene3D::Mesh *m : active_meshes)
		delete m;

	active_meshes.clear();

	uint64_t count_layers = 0, count_verts = 0, count_faces = 0;

	for(Sector *sector : sectors){
		active_meshes.push_back(sector->calculateMesh(mtcam));

		// Just used for debug output... could be removed later.
		sector->reportGeometryTotals(count_layers, count_verts, count_faces);
	}

	// FIXME debug
	cout << "calculateMesh TOTAL [layers=" << count_layers << "] [verts=" << count_verts << "] [faces=" << count_faces << "]" << endl;
}
WedgeTerrain::WedgeTerrain(Scene3D::MultiThreadCamera *cam) :
	Renderable(cam),
	mtcam(cam)
{
	for(int i = 0; i < 8; ++ i){
		for(int ii = 0; ii < 8; ++ ii){
			sectors.push_back(new Sector(i, ii, 0, ((ii*i) % 7) + 1));
		}
	}

	// Called when terrain changes.
	calculateMesh();
}
WedgeTerrain::~WedgeTerrain(){
	for(Scene3D::Mesh *mesh : active_meshes)
		delete mesh;

	for(Sector *sector : sectors)
		delete sector;
}
void WedgeTerrain::draw_if_cam(int ticks, Scene3D::Camera const *refCam){
	// We need to pass through the reference cam check to every mesh because they may be on any cam.
	for(Scene3D::Mesh *mesh : active_meshes)
		mesh->draw_if_cam(ticks, refCam);
}
