#include "Camera.h"
#include "Character.h"
#include "World.h"
#include <chrono>
#include <list>
#include <memory>
#include <random>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <windows.h>


constexpr int VIEW_W = 56;
constexpr int VIEW_H = 42;
constexpr int RADIUS = 3;
constexpr int CAPA_CHUNK_CACHE = 64;

using Key = uint64_t;
using HashChunk = std::unordered_map<Key, Chunk>;
using LruList = std::list<Key>;
using WhereMap  = std::unordered_map<Key, LruList::iterator>;


struct ChunkCache {
  HashChunk  data;     // key -> Chunk
  LruList    lru;      // 最近使用在前
  WhereMap   where;    // key -> iterator in lru
  size_t     capacity; // 64
};

struct VisibleChunks {
  // 最多 2×2
  Key keys[4];
  Chunk* ptrs[4];
  int count = 0;
};

void enableAnsi() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE) return;
  DWORD mode = 0;
  if (!GetConsoleMode(hOut, &mode)) return;
  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // 关键
  SetConsoleMode(hOut, mode);
}

/**
 *
 * @param chunk_cache
 * @param key
 *
 * If the key existed in Least Recently Used List, put it to begin
 * If not exists, add it and its iterator
 *
 * */
inline void touch(ChunkCache&chunk_cache, Key key) {
  auto it = chunk_cache.where.find(key);
  if (it != chunk_cache.where.end()) {
    //说明找到了, 移动到表头
    chunk_cache.lru.splice(chunk_cache.lru.begin(), chunk_cache.lru, it->second); // 移到表头
  } else {
    //说明不存在
    chunk_cache.lru.emplace_front(key);
    chunk_cache.where.emplace(key, chunk_cache.lru.begin());
  }
}


inline void erase_one(ChunkCache& c, Key k) {
  if (auto it = c.where.find(k); it != c.where.end()) {
    c.lru.erase(it->second);
    c.where.erase(it);
  }
  c.data.erase(k);
}

/**
 * @param cx
 * @param cy
 * Calculate an unique 64 bit key from two 32 bits integer (coordinate x, y)
 *
 * */
inline uint64_t make_chunk_key(int cx, int cy) {
  uint64_t ux = static_cast<uint32_t>(cx);
  uint64_t uy = static_cast<uint32_t>(cy);
  return (ux << 32) | uy;
}

/**
 *
 * @param key
 * Get the coordinate of chunk using its unique key
 *
 * */
inline std::pair<int,int> unpack_chunk_key(uint64_t key) {
  auto uy = static_cast<uint32_t>(key);
  auto ux = static_cast<uint32_t>(key >> 32);
  return { static_cast<int>(ux), static_cast<int>(uy) };
}

inline int floor_div(int a, int b) {
  int q = a / b;
  int r = a % b;
  if ((r != 0) && ((r > 0) != (b > 0))) --q;
  return q;
}

inline int floor_mod(int a, int b) {
  return a - b * floor_div(a, b);
}


/**
 *
 * A Rectangle with 4 coordinate:
 * [x0,x1), [y0,y1)
 *
 * */
struct Rect4 {
  int x0, y0, x1, y1;
};

/**
 *
 * Compute the rectangle of camera
 *
 * */
inline Rect4 camera_world_rect(const std::pair<float,float>&cam_loc) {
  int wx0 = static_cast<int>(std::floor(cam_loc.first - VIEW_W * 0.5f));
  int wy0 = static_cast<int>(std::floor(cam_loc.second - VIEW_H * 0.5f));
  return { wx0, wy0, wx0 + VIEW_W, wy0 + VIEW_H };
}

/**
 *
 * @param r rectangle
 *
 *
 * */
inline void compute_required_chunks(const Rect4 & r,
                                    int radius,
                                    std::vector<Key>& out) {
  // 转成 chunk 坐标范围（注意 -1，因为是半开区间）
  int cx0 = floor_div(r.x0, CHUNK_SIZE);
  int cy0 = floor_div(r.y0, CHUNK_SIZE);
  int cx1 = floor_div(r.x1 - 1, CHUNK_SIZE);
  int cy1 = floor_div(r.y1 - 1, CHUNK_SIZE);
  // 扩展 radius 圈
  int rx0 = cx0 - radius, rx1 = cx1 + radius;
  int ry0 = cy0 - radius, ry1 = cy1 + radius;

  out.clear();
  out.reserve((rx1 - rx0 + 1) * (ry1 - ry0 + 1));
  for (int cy = ry0; cy <= ry1; ++cy)
    for (int cx = rx0; cx <= rx1; ++cx)
      out.push_back(make_chunk_key(cx, cy));
}

/**
 * @param cache the cache for chunks nearby, now is empty
 * @param camera_loc the current location of camera
 * @param seed the seed of the world
 *
 *
 * Compute the chunks required to be rendered and
 * */
void initialChunks(const std::pair<float,float>& camera_loc,
                   ChunkCache& cache,
                   uint64_t seed) {


  Rect4 rect = camera_world_rect(camera_loc);

  std::vector<Key> req_keys;
  compute_required_chunks(rect, RADIUS, req_keys);

  for (Key k : req_keys) {
    // 对所有周围的chunk遍历, 寻找是否其在缓存中, 不在的话就加入
    if (!cache.data.count(k)) {
      auto [cx, cy] = unpack_chunk_key(k);
      cache.data.emplace(k, Chunk(seed, cx, cy));
    }
    //放入最近使用表中
    touch(cache, k);
  }
}

void updateChunksRendered(const std::pair<float,float>& camera_loc,
                          ChunkCache& cache,
                          uint64_t seed) {
  Rect4 rect = camera_world_rect(camera_loc);

  std::vector<Key> req_keys;
  compute_required_chunks(rect, RADIUS, req_keys);

  std::unordered_set<Key> pinned;
  pinned.reserve(req_keys.size());

  // 没有的chunk加入
  // 对所有的周围chunk, 加入pinned, 用于之后的防止驱逐
  for (Key k : req_keys) {
    if (!cache.data.count(k)) {
      auto [cx, cy] = unpack_chunk_key(k);
      cache.data.emplace(k, Chunk(seed, cx, cy));
    }
    touch(cache, k);
    pinned.insert(k);
  }

  // evict (跳过 pinned)
  while (cache.data.size() > cache.capacity && !cache.lru.empty()) {
    // 找到最靠后的非 pinned
    auto it = cache.lru.end();
    bool removed = false;
    while (it != cache.lru.begin()) {
      --it;
      if (!pinned.count(*it)) {
        erase_one(cache, *it);
        removed = true;
        break;
      }
    }
    if (!removed) break; // 全是 pinned，先不驱逐
  }
}


std::unique_ptr<World> generateWorld(uint64_t seed){
  std::unique_ptr<World> world = std::make_unique<World>(seed);
  return world;
}

long long random_seed(){
  std::mt19937 myrand(time(nullptr));
  return myrand();
}

VisibleChunks compute_visible_chunks(const Rect4& rect, ChunkCache& cache) {
  VisibleChunks vc{};
  int cx0 = floor_div(rect.x0, CHUNK_SIZE);
  int cy0 = floor_div(rect.y0, CHUNK_SIZE);
  int cx1 = floor_div(rect.x1 - 1, CHUNK_SIZE);
  int cy1 = floor_div(rect.y1 - 1, CHUNK_SIZE);
  for (int cy = cy0; cy <= cy1; ++cy)
    for (int cx = cx0; cx <= cx1; ++cx) {
      Key k = make_chunk_key(cx, cy);
      auto it = cache.data.find(k);
      vc.keys[vc.count] = k;
      vc.ptrs[vc.count] = &it->second;
      ++vc.count;
    }
  return vc;
}

inline char tile_glyph(TileType t) {
  switch (t) {
    case TileType::Sea:  return '~';
    case TileType::Rock: return '^';
    case TileType::Grass:
    default:             return '.';
  }
}



void render_frame(const Rect4& rect,
                  const Character&character,
                  ChunkCache& cache,
                  uint64_t seed) {
  // 1) 预取视口覆盖的 chunk 指针（最多 4 个）
  VisibleChunks vc = compute_visible_chunks(rect, cache);

  // 2) 准备帧缓冲（W 行，每行 W 字符 + '\n'）
  std::string frame;
  frame.resize(VIEW_H * (VIEW_W + 1));

  auto buf_at = [&](int sx, int sy) -> char& {
    return frame[ sy * (VIEW_W + 1) + sx ];
  };

  for (int y = 0; y < VIEW_H; ++y) frame[y*(VIEW_W+1) + VIEW_W] = '\n';


  int wx0 = rect.x0, wy0 = rect.y0;
  for (int sy = 0; sy < VIEW_H; ++sy) {
    int wy = wy0 + sy;

    int cy = floor_div(wy, CHUNK_SIZE);
    int ly = floor_mod(wy, CHUNK_SIZE);
    int cx = floor_div(wx0, CHUNK_SIZE);
    int lx = floor_mod(wx0, CHUNK_SIZE);

    for (int sx = 0; sx < VIEW_W; ++sx) {

      Key k = make_chunk_key(cx, cy);
      Chunk* chunk = nullptr;
      for (int i = 0; i < vc.count; ++i) if (vc.keys[i] == k) { chunk = vc.ptrs[i]; break; }

      const Block& b = chunk->at(lx, ly);
      buf_at(sx, sy) = tile_glyph((TileType)b._tile_type);

      // 向右走一格：递增 lx / 跨 chunk
      if (++lx == CHUNK_SIZE) { lx = 0; ++cx; }
    }
  }

  // 4) 覆盖玩家 '@'
  int sx = static_cast<int>(std::floor(character.getLoc().first  - rect.x0));
  int sy = static_cast<int>(std::floor(character.getLoc().second - rect.y0));
  if (0 <= sx && sx < VIEW_W && 0 <= sy && sy < VIEW_H) {
    buf_at(sx, sy) = '@';
  }
  //system("cls");
  // 5) 输出（ANSI：光标归位再覆盖整帧）
  //std::cout << frame << std::flush;   // 如果启动时清过屏

  // 或首次渲染前清屏一次
  static bool first = true;
  if (first) { std::cout << "\x1b[2J\x1b[H"; first = false; }
  std::cout << "\x1b[H" << frame << "Seed: " << seed <<"\n"
            << "Character location: ("<< character.getLoc().first <<", " << character.getLoc().second <<")\n"<<std::flush;
}


int main() {

  enableAnsi();

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  Camera camera;
  uint64_t seed = random_seed();
  std::unique_ptr<World> world = generateWorld(seed);
  bool running = true;
  Character character{};
  camera.onUpdate({1.0f,1.0f});

  HashChunk hash_chunk;

  ChunkCache chunk_cache;
  chunk_cache.capacity = CAPA_CHUNK_CACHE;
  initialChunks(camera.getPos(), chunk_cache, seed);

  //world->print_seed();


  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(33);
  while(running){

    //移动角色
    character.move();
    //移动镜头
    camera.onUpdate(character.getLoc());
    //判断是否有新chunk加入
    //更新cachechunk和lastusedkeys
    updateChunksRendered(camera.getPos(), chunk_cache, seed);

    //用镜头坐标求要渲染的坐标的世界坐标
    auto rect_4 = camera_world_rect(camera.getPos());
    //根据cachechunk找到其chunk, 再找到其block
    //渲染
    //渲染其他物品
    //渲染角色
    render_frame(rect_4, character,chunk_cache, seed);
    next_tick += dt;
    std::this_thread::sleep_until(next_tick);

  }

}

