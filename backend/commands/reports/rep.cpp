#include "commands/reports/rep.h"
#include "disk/mbr.h"
#include "disk/partition.h"
#include "disk/ebr.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include "filesystem/blocks.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <filesystem>

namespace extreamfs {
namespace commands {

namespace {

/** Escapa caracteres especiales para uso dentro de tabla HTML en Graphviz. */
static std::string htmlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '&') out += "&amp;";
        else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '"') out += "&quot;";
        else out += c;
    }
    return out;
}

/** Convierte char[N] a string (hasta null o N caracteres). */
static std::string charArrayToString(const char* buf, size_t maxLen) {
    std::string s;
    for (size_t i = 0; i < maxLen && buf[i] != '\0'; ++i)
        s += buf[i];
    return s;
}

/** Formatea time_t como DD/MM/YYYY HH:MM:SS. */
static std::string formatDate(time_t t) {
    if (t == 0) return "0";
    struct tm* tm = std::localtime(&t);
    if (!tm) return "?";
    char buf[32];
    std::strftime(buf, sizeof(buf), "%d/%m/%Y %H:%M:%S", tm);
    return std::string(buf);
}

/** Formatea entero como fecha (timestamp) DD/MM/YYYY HH:MM:SS. */
static std::string formatDateFromInt(int t) {
    return formatDate(static_cast<time_t>(t));
}

/** Devuelve extensión del archivo en minúsculas (ej: .png, .jpg, .pdf). */
static std::string getExtension(const std::string& path) {
    std::filesystem::path p(path);
    std::string ext = p.extension().string();
    for (char& c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return ext;
}

/** Convierte contenido de FileBlock (64 bytes) a string mostrable: imprimibles o '.'. */
static std::string fileBlockContentToString(const char* content, size_t len) {
    std::string s;
    s.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = static_cast<unsigned char>(content[i]);
        s += (c >= 32 && c < 127) ? static_cast<char>(c) : '.';
    }
    return s;
}

/** i_perm[3] a string tipo "664". */
static std::string permToString(const char p[3]) {
    std::string s;
    for (int i = 0; i < 3; ++i)
        s += (p[i] != 0) ? p[i] : '0';
    return s;
}

/** Compara Content.b_name con name (para búsqueda en FolderBlock). */
static bool contentNameEquals(const Content& c, const std::string& name) {
    size_t i = 0;
    const size_t maxName = 12;
    while (i < maxName && c.b_name[i] != '\0' && i < name.size()) {
        if (c.b_name[i] != name[i]) return false;
        ++i;
    }
    if (i < name.size()) return false;
    if (i < maxName && c.b_name[i] != '\0') return false;
    return true;
}

/** Divide ruta en componentes (ej: /home/users.txt -> ["home","users.txt"]). */
static std::vector<std::string> splitPathComponents(const std::string& ruta) {
    std::vector<std::string> out;
    std::string s = ruta;
    while (!s.empty() && s[0] == '/') s.erase(0, 1);
    if (s.empty()) return out;
    std::istringstream iss(s);
    std::string part;
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) out.push_back(part);
    }
    return out;
}

/** Resuelve ruta absoluta al índice del inodo del último componente. -1 = no encontrado, -2 = error. */
static int resolvePathToInode(const std::string& diskPath, int part_start, const Superblock& sb,
                              const std::string& ruta, std::string& outError) {
    std::vector<std::string> components = splitPathComponents(ruta);
    if (components.empty()) return -1;
    const int maxDirectBlocks = 12;
    int currentIdx = 0;
    Inode current;
    for (size_t i = 0; i + 1 < components.size(); ++i) {
        if (!manager::readInode(diskPath, part_start, sb, currentIdx, current, outError))
            return -2;
        if (current.i_type != 0) return -1;
        int childIdx = -1;
        for (int b = 0; b < maxDirectBlocks && current.i_block[b] >= 0; ++b) {
            FolderBlock fb;
            if (!manager::readFolderBlock(diskPath, part_start, sb, current.i_block[b], fb, outError))
                return -2;
            for (int e = 0; e < 4; ++e) {
                if (fb.b_content[e].b_inodo >= 0 && contentNameEquals(fb.b_content[e], components[i])) {
                    childIdx = fb.b_content[e].b_inodo;
                    break;
                }
            }
            if (childIdx >= 0) break;
        }
        if (childIdx < 0) return -1;
        currentIdx = childIdx;
    }
    if (!manager::readInode(diskPath, part_start, sb, currentIdx, current, outError))
        return -2;
    if (current.i_type != 0) return -1;
    const std::string& fileName = components.back();
    for (int b = 0; b < maxDirectBlocks && current.i_block[b] >= 0; ++b) {
        FolderBlock fb;
        if (!manager::readFolderBlock(diskPath, part_start, sb, current.i_block[b], fb, outError))
            return -2;
        for (int e = 0; e < 4; ++e) {
            if (fb.b_content[e].b_inodo >= 0 && contentNameEquals(fb.b_content[e], fileName))
                return fb.b_content[e].b_inodo;
        }
    }
    return -1;
}

} // anonymous namespace

/** Genera reporte MBR con particiones y cadena de EBR si existe extendida. */
static bool generateMbrReport(const std::string& diskPath, const std::string& outputPath, std::string& outError) {
    std::ifstream disk(diskPath, std::ios::binary);
    if (!disk.is_open()) {
        outError = "No se puede abrir el disco.";
        return false;
    }
    MBR mbr;
    disk.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    if (!disk) {
        outError = "No se pudo leer el MBR.";
        return false;
    }

    std::ostringstream dot;
    dot << "digraph G {\n";
    dot << "    node [shape=plaintext]\n";
    dot << "    mbr [label=<\n";
    dot << "        <table border=\"1\" cellborder=\"1\">\n";

    dot << "            <tr><td colspan=\"2\"><b>MBR</b></td></tr>\n";
    dot << "            <tr><td>mbr_tamano</td><td>" << mbr.mbr_tamano << "</td></tr>\n";
    dot << "            <tr><td>mbr_fecha_creacion</td><td>" << htmlEscape(formatDate(mbr.mbr_fecha_creacion)) << "</td></tr>\n";
    dot << "            <tr><td>mbr_dsk_signature</td><td>" << mbr.mbr_dsk_signature << "</td></tr>\n";
    dot << "            <tr><td>dsk_fit</td><td>" << (mbr.dsk_fit ? htmlEscape(std::string(1, mbr.dsk_fit)) : "-") << "</td></tr>\n";

    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (p.part_s <= 0) continue;
        dot << "            <tr><td colspan=\"2\"><b>PARTICION " << (i + 1) << "</b></td></tr>\n";
        dot << "            <tr><td>part_status</td><td>" << (p.part_status ? static_cast<int>(static_cast<unsigned char>(p.part_status)) : 0) << "</td></tr>\n";
        dot << "            <tr><td>part_type</td><td>" << (p.part_type ? htmlEscape(std::string(1, p.part_type)) : "-") << "</td></tr>\n";
        dot << "            <tr><td>part_fit</td><td>" << (p.part_fit ? htmlEscape(std::string(1, p.part_fit)) : "-") << "</td></tr>\n";
        dot << "            <tr><td>part_start</td><td>" << p.part_start << "</td></tr>\n";
        dot << "            <tr><td>part_s</td><td>" << p.part_s << "</td></tr>\n";
        dot << "            <tr><td>part_name</td><td>" << htmlEscape(charArrayToString(p.part_name, 16)) << "</td></tr>\n";
        dot << "            <tr><td>part_correlative</td><td>" << p.part_correlative << "</td></tr>\n";
        dot << "            <tr><td>part_id</td><td>" << htmlEscape(charArrayToString(p.part_id, 4)) << "</td></tr>\n";

        if (p.part_type == 'E' || p.part_type == 'e') {
            int nextEbrPos = p.part_start;
            int ebrIndex = 0;
            while (nextEbrPos >= 0) {
                disk.seekg(nextEbrPos);
                EBR ebr;
                disk.read(reinterpret_cast<char*>(&ebr), sizeof(EBR));
                if (!disk) break;
                dot << "            <tr><td colspan=\"2\"><b>EBR " << (ebrIndex + 1) << "</b></td></tr>\n";
                dot << "            <tr><td>part_mount</td><td>" << (ebr.part_mount ? static_cast<int>(static_cast<unsigned char>(ebr.part_mount)) : 0) << "</td></tr>\n";
                dot << "            <tr><td>part_fit</td><td>" << (ebr.part_fit ? htmlEscape(std::string(1, ebr.part_fit)) : "-") << "</td></tr>\n";
                dot << "            <tr><td>part_start</td><td>" << ebr.part_start << "</td></tr>\n";
                dot << "            <tr><td>part_s</td><td>" << ebr.part_s << "</td></tr>\n";
                dot << "            <tr><td>part_next</td><td>" << ebr.part_next << "</td></tr>\n";
                dot << "            <tr><td>part_name</td><td>" << htmlEscape(charArrayToString(ebr.part_name, 16)) << "</td></tr>\n";
                nextEbrPos = ebr.part_next;
                if (nextEbrPos == -1) break;
                ++ebrIndex;
            }
        }
    }

    dot << "        </table>\n";
    dot << "    >];\n";
    dot << "}\n";

    disk.close();

    std::filesystem::path out(outputPath);
    std::filesystem::path parent = out.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            outError = "No se pudo crear el directorio de salida.";
            return false;
        }
    }

    std::filesystem::path dotPath = parent / (out.stem().string() + ".dot");
    if (dotPath.empty()) dotPath = std::filesystem::path(out.stem().string() + ".dot");
    {
        std::ofstream dotFile(dotPath.string());
        if (!dotFile.is_open()) {
            outError = "No se pudo crear el archivo DOT.";
            return false;
        }
        dotFile << dot.str();
    }

    std::string ext = getExtension(outputPath);
    std::string format = "png";
    if (ext == ".jpg" || ext == ".jpeg") format = "jpg";
    else if (ext == ".pdf") format = "pdf";
    else format = "png";

    std::string cmd = "dot -T" + format + " \"" + dotPath.string() + "\" -o \"" + outputPath + "\"";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        outError = "Error al ejecutar Graphviz (dot). Verifique que esté instalado.";
        return false;
    }
    return true;
}

/** Genera reporte del Superblock de la partición montada. */
static bool generateSbReport(const std::string& diskPath, const std::string& partName, const std::string& outputPath, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición.";
        return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque.";
        return false;
    }
    const int EXT2_MAGIC = 0xEF53;
    if (sb.s_magic != EXT2_MAGIC) {
        outError = "s_magic inválido (no es partición EXT2).";
        return false;
    }

    std::ostringstream dot;
    dot << "digraph G {\n";
    dot << "    node [shape=plaintext]\n";
    dot << "    sb [label=<\n";
    dot << "        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"2\"><b>SUPERBLOCK</b></td></tr>\n";
    dot << "            <tr><td>s_filesystem_type</td><td>" << sb.s_filesystem_type << "</td></tr>\n";
    dot << "            <tr><td>s_inodes_count</td><td>" << sb.s_inodes_count << "</td></tr>\n";
    dot << "            <tr><td>s_blocks_count</td><td>" << sb.s_blocks_count << "</td></tr>\n";
    dot << "            <tr><td>s_free_blocks_count</td><td>" << sb.s_free_blocks_count << "</td></tr>\n";
    dot << "            <tr><td>s_free_inodes_count</td><td>" << sb.s_free_inodes_count << "</td></tr>\n";
    dot << "            <tr><td>s_mtime</td><td>" << htmlEscape(formatDateFromInt(sb.s_mtime)) << "</td></tr>\n";
    dot << "            <tr><td>s_umtime</td><td>" << htmlEscape(formatDateFromInt(sb.s_umtime)) << "</td></tr>\n";
    dot << "            <tr><td>s_mnt_count</td><td>" << sb.s_mnt_count << "</td></tr>\n";
    dot << "            <tr><td>s_magic</td><td>" << sb.s_magic << "</td></tr>\n";
    dot << "            <tr><td>s_inode_s</td><td>" << sb.s_inode_s << "</td></tr>\n";
    dot << "            <tr><td>s_block_s</td><td>" << sb.s_block_s << "</td></tr>\n";
    dot << "            <tr><td>s_firts_ino</td><td>" << sb.s_firts_ino << "</td></tr>\n";
    dot << "            <tr><td>s_first_blo</td><td>" << sb.s_first_blo << "</td></tr>\n";
    dot << "            <tr><td>s_bm_inode_start</td><td>" << sb.s_bm_inode_start << "</td></tr>\n";
    dot << "            <tr><td>s_bm_block_start</td><td>" << sb.s_bm_block_start << "</td></tr>\n";
    dot << "            <tr><td>s_inode_start</td><td>" << sb.s_inode_start << "</td></tr>\n";
    dot << "            <tr><td>s_block_start</td><td>" << sb.s_block_start << "</td></tr>\n";
    dot << "        </table>\n";
    dot << "    >];\n";
    dot << "}\n";

    std::filesystem::path out(outputPath);
    std::filesystem::path parent = out.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            outError = "No se pudo crear el directorio de salida.";
            return false;
        }
    }

    std::filesystem::path dotPath = parent.empty() ? std::filesystem::path(out.stem().string() + ".dot") : parent / (out.stem().string() + ".dot");
    {
        std::ofstream dotFile(dotPath.string());
        if (!dotFile.is_open()) {
            outError = "No se pudo crear el archivo DOT.";
            return false;
        }
        dotFile << dot.str();
    }

    std::string ext = getExtension(outputPath);
    std::string format = "png";
    if (ext == ".jpg" || ext == ".jpeg") format = "jpg";
    else if (ext == ".pdf") format = "pdf";

    std::string cmd = "dot -T" + format + " \"" + dotPath.string() + "\" -o \"" + outputPath + "\"";
    if (std::system(cmd.c_str()) != 0) {
        outError = "Error al ejecutar Graphviz (dot). Verifique que esté instalado.";
        return false;
    }
    return true;
}

/** Constante: celdas por fila en el reporte del bitmap (formato común del proyecto). */
static const int BM_INODE_COLS = 20;

/** Genera reporte del Bitmap de Inodos. Lee bytes desde disco, 20 valores por fila. */
static bool generateBmInodeReport(const std::string& diskPath, const std::string& partName, const std::string& outputPath, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición.";
        return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque.";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "s_magic inválido (no es partición EXT2).";
        return false;
    }

    const int n = sb.s_inodes_count;
    std::vector<char> bm(static_cast<size_t>(n), 0);
    {
        std::ifstream file(diskPath, std::ios::binary);
        if (!file) {
            outError = "No se puede abrir el disco.";
            return false;
        }
        file.seekg(part_start + sb.s_bm_inode_start);
        file.read(bm.data(), n);
        if (!file || file.gcount() != static_cast<std::streamsize>(n)) {
            outError = "No se puede leer el bitmap de inodos.";
            return false;
        }
    }

    std::ostringstream dot;
    dot << "digraph G {\n";
    dot << "    node [shape=plaintext]\n";
    dot << "    bm_inode [label=<\n";
    dot << "        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"" << BM_INODE_COLS << "\"><b>BITMAP INODE</b></td></tr>\n";

    for (int i = 0; i < n; i += BM_INODE_COLS) {
        dot << "            <tr>\n";
        for (int j = 0; j < BM_INODE_COLS && (i + j) < n; ++j) {
            int v = (bm[static_cast<size_t>(i + j)] != 0) ? 1 : 0;
            dot << "<td>" << v << "</td>";
        }
        dot << "\n            </tr>\n";
    }

    dot << "        </table>\n";
    dot << "    >];\n";
    dot << "}\n";

    std::filesystem::path out(outputPath);
    std::filesystem::path parent = out.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            outError = "No se pudo crear el directorio de salida.";
            return false;
        }
    }

    std::filesystem::path dotPath = parent.empty() ? std::filesystem::path(out.stem().string() + ".dot") : parent / (out.stem().string() + ".dot");
    {
        std::ofstream dotFile(dotPath.string());
        if (!dotFile.is_open()) {
            outError = "No se pudo crear el archivo DOT.";
            return false;
        }
        dotFile << dot.str();
    }

    std::string ext = getExtension(outputPath);
    std::string format = "png";
    if (ext == ".jpg" || ext == ".jpeg") format = "jpg";
    else if (ext == ".pdf") format = "pdf";

    std::string cmd = "dot -T" + format + " \"" + dotPath.string() + "\" -o \"" + outputPath + "\"";
    if (std::system(cmd.c_str()) != 0) {
        outError = "Error al ejecutar Graphviz (dot). Verifique que esté instalado.";
        return false;
    }
    return true;
}

/** Constante: celdas por fila en el reporte del bitmap de bloques. */
static const int BM_BLOCK_COLS = 20;

/** Genera reporte del Bitmap de Bloques. Lee bytes desde disco, 20 valores por fila. */
static bool generateBmBlockReport(const std::string& diskPath, const std::string& partName, const std::string& outputPath, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición.";
        return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque.";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "s_magic inválido (no es partición EXT2).";
        return false;
    }

    const int n = sb.s_blocks_count;
    std::vector<char> bm(static_cast<size_t>(n), 0);
    {
        std::ifstream file(diskPath, std::ios::binary);
        if (!file) {
            outError = "No se puede abrir el disco.";
            return false;
        }
        file.seekg(part_start + sb.s_bm_block_start);
        file.read(bm.data(), n);
        if (!file || file.gcount() != static_cast<std::streamsize>(n)) {
            outError = "No se puede leer el bitmap de bloques.";
            return false;
        }
    }

    std::ostringstream dot;
    dot << "digraph G {\n";
    dot << "    node [shape=plaintext]\n";
    dot << "    bm_block [label=<\n";
    dot << "        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"" << BM_BLOCK_COLS << "\"><b>BITMAP BLOCK</b></td></tr>\n";

    for (int i = 0; i < n; i += BM_BLOCK_COLS) {
        dot << "            <tr>\n";
        for (int j = 0; j < BM_BLOCK_COLS && (i + j) < n; ++j) {
            int v = (bm[static_cast<size_t>(i + j)] != 0) ? 1 : 0;
            dot << "<td>" << v << "</td>";
        }
        dot << "\n            </tr>\n";
    }

    dot << "        </table>\n";
    dot << "    >];\n";
    dot << "}\n";

    std::filesystem::path out(outputPath);
    std::filesystem::path parent = out.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            outError = "No se pudo crear el directorio de salida.";
            return false;
        }
    }

    std::filesystem::path dotPath = parent.empty() ? std::filesystem::path(out.stem().string() + ".dot") : parent / (out.stem().string() + ".dot");
    {
        std::ofstream dotFile(dotPath.string());
        if (!dotFile.is_open()) {
            outError = "No se pudo crear el archivo DOT.";
            return false;
        }
        dotFile << dot.str();
    }

    std::string ext = getExtension(outputPath);
    std::string format = "png";
    if (ext == ".jpg" || ext == ".jpeg") format = "jpg";
    else if (ext == ".pdf") format = "pdf";

    std::string cmd = "dot -T" + format + " \"" + dotPath.string() + "\" -o \"" + outputPath + "\"";
    if (std::system(cmd.c_str()) != 0) {
        outError = "Error al ejecutar Graphviz (dot). Verifique que esté instalado.";
        return false;
    }
    return true;
}

/** Escribe DOT en archivo, ejecuta dot y guarda imagen. Reutilizado por reportes. */
static bool writeDotAndExport(const std::string& dotContent, const std::string& outputPath, std::string& outError) {
    std::filesystem::path out(outputPath);
    std::filesystem::path parent = out.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            outError = "No se pudo crear el directorio de salida.";
            return false;
        }
    }
    std::filesystem::path dotPath = parent.empty() ? std::filesystem::path(out.stem().string() + ".dot") : parent / (out.stem().string() + ".dot");
    std::ofstream dotFile(dotPath.string());
    if (!dotFile.is_open()) {
        outError = "No se pudo crear el archivo DOT.";
        return false;
    }
    dotFile << dotContent;
    std::string ext = getExtension(outputPath);
    std::string format = "png";
    if (ext == ".jpg" || ext == ".jpeg") format = "jpg";
    else if (ext == ".pdf") format = "pdf";
    std::string cmd = "dot -T" + format + " \"" + dotPath.string() + "\" -o \"" + outputPath + "\"";
    if (std::system(cmd.c_str()) != 0) {
        outError = "Error al ejecutar Graphviz (dot). Verifique que esté instalado.";
        return false;
    }
    return true;
}

/** Genera reporte DISK: estructura del disco (MBR + particiones + EBR), con ruta del disco. */
static bool generateDiskReport(const std::string& diskPath, const std::string& outputPath, std::string& outError) {
    std::ifstream disk(diskPath, std::ios::binary);
    if (!disk.is_open()) {
        outError = "No se puede abrir el disco.";
        return false;
    }
    MBR mbr;
    disk.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    if (!disk) {
        outError = "No se pudo leer el MBR.";
        return false;
    }
    std::ostringstream dot;
    dot << "digraph G {\n    node [shape=plaintext]\n    disk [label=<\n        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"2\"><b>REPORTE DISK</b></td></tr>\n";
    dot << "            <tr><td>Ruta disco</td><td>" << htmlEscape(diskPath) << "</td></tr>\n";
    dot << "            <tr><td colspan=\"2\"><b>MBR</b></td></tr>\n";
    dot << "            <tr><td>mbr_tamano</td><td>" << mbr.mbr_tamano << "</td></tr>\n";
    dot << "            <tr><td>mbr_fecha_creacion</td><td>" << htmlEscape(formatDate(mbr.mbr_fecha_creacion)) << "</td></tr>\n";
    dot << "            <tr><td>mbr_dsk_signature</td><td>" << mbr.mbr_dsk_signature << "</td></tr>\n";
    dot << "            <tr><td>dsk_fit</td><td>" << (mbr.dsk_fit ? htmlEscape(std::string(1, mbr.dsk_fit)) : "-") << "</td></tr>\n";
    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (p.part_s <= 0) continue;
        dot << "            <tr><td colspan=\"2\"><b>PARTICION " << (i + 1) << "</b></td></tr>\n";
        dot << "            <tr><td>part_status</td><td>" << (p.part_status ? static_cast<int>(static_cast<unsigned char>(p.part_status)) : 0) << "</td></tr>\n";
        dot << "            <tr><td>part_type</td><td>" << (p.part_type ? htmlEscape(std::string(1, p.part_type)) : "-") << "</td></tr>\n";
        dot << "            <tr><td>part_fit</td><td>" << (p.part_fit ? htmlEscape(std::string(1, p.part_fit)) : "-") << "</td></tr>\n";
        dot << "            <tr><td>part_start</td><td>" << p.part_start << "</td></tr>\n";
        dot << "            <tr><td>part_s</td><td>" << p.part_s << "</td></tr>\n";
        dot << "            <tr><td>part_name</td><td>" << htmlEscape(charArrayToString(p.part_name, 16)) << "</td></tr>\n";
        dot << "            <tr><td>part_correlative</td><td>" << p.part_correlative << "</td></tr>\n";
        dot << "            <tr><td>part_id</td><td>" << htmlEscape(charArrayToString(p.part_id, 4)) << "</td></tr>\n";
        if (p.part_type == 'E' || p.part_type == 'e') {
            int nextEbrPos = p.part_start;
            int ebrIndex = 0;
            while (nextEbrPos >= 0) {
                disk.seekg(nextEbrPos);
                EBR ebr;
                disk.read(reinterpret_cast<char*>(&ebr), sizeof(EBR));
                if (!disk) break;
                dot << "            <tr><td colspan=\"2\"><b>EBR " << (ebrIndex + 1) << "</b></td></tr>\n";
                dot << "            <tr><td>part_mount</td><td>" << (ebr.part_mount ? static_cast<int>(static_cast<unsigned char>(ebr.part_mount)) : 0) << "</td></tr>\n";
                dot << "            <tr><td>part_fit</td><td>" << (ebr.part_fit ? htmlEscape(std::string(1, ebr.part_fit)) : "-") << "</td></tr>\n";
                dot << "            <tr><td>part_start</td><td>" << ebr.part_start << "</td></tr>\n";
                dot << "            <tr><td>part_s</td><td>" << ebr.part_s << "</td></tr>\n";
                dot << "            <tr><td>part_next</td><td>" << ebr.part_next << "</td></tr>\n";
                dot << "            <tr><td>part_name</td><td>" << htmlEscape(charArrayToString(ebr.part_name, 16)) << "</td></tr>\n";
                nextEbrPos = ebr.part_next;
                if (nextEbrPos == -1) break;
                ++ebrIndex;
            }
        }
    }
    dot << "        </table>\n    >];\n}\n";
    disk.close();
    return writeDotAndExport(dot.str(), outputPath, outError);
}

/** Genera reporte INODE: tabla del inodo indicado por -ruta. */
static bool generateInodeReport(const std::string& diskPath, const std::string& partName,
                                const std::string& outputPath, const std::string& ruta, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición.";
        return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque.";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "s_magic inválido (no es partición EXT2).";
        return false;
    }
    int inodeIdx = resolvePathToInode(diskPath, part_start, sb, ruta, outError);
    if (inodeIdx == -2) { if (outError.empty()) outError = "Error al leer."; return false; }
    if (inodeIdx == -1) { outError = "No se encontró la ruta especificada."; return false; }
    Inode ino;
    if (!manager::readInode(diskPath, part_start, sb, inodeIdx, ino, outError)) {
        if (outError.empty()) outError = "No se puede leer el inodo."; return false;
    }
    std::ostringstream dot;
    dot << "digraph G {\n    node [shape=plaintext]\n    inode [label=<\n        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"2\"><b>REPORTE INODO " << inodeIdx << "</b></td></tr>\n";
    dot << "            <tr><td>Ruta</td><td>" << htmlEscape(ruta) << "</td></tr>\n";
    dot << "            <tr><td>i_uid</td><td>" << ino.i_uid << "</td></tr>\n";
    dot << "            <tr><td>i_gid</td><td>" << ino.i_gid << "</td></tr>\n";
    dot << "            <tr><td>i_s</td><td>" << ino.i_s << "</td></tr>\n";
    dot << "            <tr><td>i_atime</td><td>" << htmlEscape(formatDateFromInt(ino.i_atime)) << "</td></tr>\n";
    dot << "            <tr><td>i_ctime</td><td>" << htmlEscape(formatDateFromInt(ino.i_ctime)) << "</td></tr>\n";
    dot << "            <tr><td>i_mtime</td><td>" << htmlEscape(formatDateFromInt(ino.i_mtime)) << "</td></tr>\n";
    dot << "            <tr><td>i_type</td><td>" << (ino.i_type == 0 ? "0" : "1") << "</td></tr>\n";
    dot << "            <tr><td>i_perm</td><td>" << htmlEscape(permToString(ino.i_perm)) << "</td></tr>\n";
    for (int i = 0; i < 15; ++i) dot << "            <tr><td>i_block[" << i << "]</td><td>" << ino.i_block[i] << "</td></tr>\n";
    dot << "        </table>\n    >];\n}\n";
    return writeDotAndExport(dot.str(), outputPath, outError);
}

/** Genera reporte BLOCK: bloques del archivo/carpeta indicado por -ruta. */
static bool generateBlockReport(const std::string& diskPath, const std::string& partName,
                                const std::string& outputPath, const std::string& ruta, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición."; return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque."; return false;
    }
    if (sb.s_magic != 0xEF53) { outError = "s_magic inválido."; return false; }
    int inodeIdx = resolvePathToInode(diskPath, part_start, sb, ruta, outError);
    if (inodeIdx == -2) { if (outError.empty()) outError = "Error al leer."; return false; }
    if (inodeIdx == -1) { outError = "No se encontró la ruta."; return false; }
    Inode ino;
    if (!manager::readInode(diskPath, part_start, sb, inodeIdx, ino, outError)) {
        if (outError.empty()) outError = "No se puede leer el inodo."; return false;
    }
    std::ostringstream dot;
    dot << "digraph G {\n    node [shape=plaintext]\n    blockReport [label=<\n        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"2\"><b>REPORTE BLOCK</b></td></tr>\n";
    dot << "            <tr><td>Ruta</td><td>" << htmlEscape(ruta) << "</td></tr>\n";
    for (int i = 0; i < 15; ++i) {
        if (ino.i_block[i] < 0) break;
        int bi = ino.i_block[i];
        if (ino.i_type == 0) {
            FolderBlock fb;
            if (!manager::readFolderBlock(diskPath, part_start, sb, bi, fb, outError)) {
                if (outError.empty()) outError = "No se puede leer el bloque."; return false;
            }
            dot << "            <tr><td colspan=\"2\"><b>BLOCK " << bi << "</b></td></tr>\n";
            dot << "            <tr><td>name</td><td>inode</td></tr>\n";
            for (int e = 0; e < 4; ++e)
                dot << "            <tr><td>" << htmlEscape(charArrayToString(fb.b_content[e].b_name, 12)) << "</td><td>" << fb.b_content[e].b_inodo << "</td></tr>\n";
        } else {
            FileBlock fblk;
            if (!manager::readFileBlock(diskPath, part_start, sb, bi, fblk, outError)) {
                if (outError.empty()) outError = "No se puede leer el bloque."; return false;
            }
            std::string content = fileBlockContentToString(fblk.b_content, 64);
            dot << "            <tr><td colspan=\"2\"><b>BLOCK " << bi << "</b></td></tr>\n";
            dot << "            <tr><td colspan=\"2\">" << htmlEscape(content) << "</td></tr>\n";
        }
    }
    dot << "        </table>\n    >];\n}\n";
    return writeDotAndExport(dot.str(), outputPath, outError);
}

/** Genera reporte LS: listado del directorio indicado por -ruta. */
static bool generateLsReport(const std::string& diskPath, const std::string& partName,
                             const std::string& outputPath, const std::string& ruta, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición."; return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque."; return false;
    }
    if (sb.s_magic != 0xEF53) { outError = "s_magic inválido."; return false; }
    int inodeIdx = resolvePathToInode(diskPath, part_start, sb, ruta, outError);
    if (inodeIdx == -2) { if (outError.empty()) outError = "Error al leer."; return false; }
    if (inodeIdx == -1) { outError = "No se encontró la ruta."; return false; }
    Inode ino;
    if (!manager::readInode(diskPath, part_start, sb, inodeIdx, ino, outError)) {
        if (outError.empty()) outError = "No se puede leer el inodo."; return false;
    }
    if (ino.i_type != 0) { outError = "La ruta no es un directorio."; return false; }
    std::ostringstream dot;
    dot << "digraph G {\n    node [shape=plaintext]\n    lsReport [label=<\n        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td colspan=\"2\"><b>REPORTE LS</b></td></tr>\n";
    dot << "            <tr><td>Ruta</td><td>" << htmlEscape(ruta) << "</td></tr>\n";
    dot << "            <tr><td>name</td><td>inode</td></tr>\n";
    for (int b = 0; b < 12 && ino.i_block[b] >= 0; ++b) {
        FolderBlock fb;
        if (!manager::readFolderBlock(diskPath, part_start, sb, ino.i_block[b], fb, outError)) {
            if (outError.empty()) outError = "No se puede leer el bloque."; return false;
        }
        for (int e = 0; e < 4; ++e) {
            if (fb.b_content[e].b_inodo < 0) continue;
            std::string name = charArrayToString(fb.b_content[e].b_name, 12);
            dot << "            <tr><td>" << htmlEscape(name) << "</td><td>" << fb.b_content[e].b_inodo << "</td></tr>\n";
        }
    }
    dot << "        </table>\n    >];\n}\n";
    return writeDotAndExport(dot.str(), outputPath, outError);
}

/** Genera reporte FILE: contenido del archivo indicado por -ruta. */
static bool generateFileReport(const std::string& diskPath, const std::string& partName,
                               const std::string& outputPath, const std::string& ruta, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición.";
        return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque.";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "s_magic inválido (no es partición EXT2).";
        return false;
    }

    int fileInodeIdx = resolvePathToInode(diskPath, part_start, sb, ruta, outError);
    if (fileInodeIdx == -2) {
        if (outError.empty()) outError = "Error al leer inodo o bloque.";
        return false;
    }
    if (fileInodeIdx == -1) {
        outError = "No se encontró el archivo especificado.";
        return false;
    }

    Inode fileInode;
    if (!manager::readInode(diskPath, part_start, sb, fileInodeIdx, fileInode, outError)) {
        if (outError.empty()) outError = "No se puede leer el inodo.";
        return false;
    }
    if (fileInode.i_type != 1) {
        outError = "El path corresponde a directorio.";
        return false;
    }

    std::string content;
    content.reserve(static_cast<size_t>(fileInode.i_s > 0 ? fileInode.i_s : 0));
    int remaining = fileInode.i_s;
    for (int i = 0; i < 15 && remaining > 0; ++i) {
        if (fileInode.i_block[i] < 0) break;
        FileBlock fblk;
        if (!manager::readFileBlock(diskPath, part_start, sb, fileInode.i_block[i], fblk, outError)) {
            if (outError.empty()) outError = "No se puede leer el bloque.";
            return false;
        }
        size_t toTake = static_cast<size_t>(std::min(64, remaining));
        content.append(fblk.b_content, toTake);
        remaining -= static_cast<int>(toTake);
    }

    std::string displayContent;
    displayContent.reserve(content.size());
    for (unsigned char c : content) {
        if (c >= 32 && c < 127)
            displayContent += static_cast<char>(c);
        else
            displayContent += '.';
    }

    std::ostringstream dot;
    dot << "digraph G {\n";
    dot << "    node [shape=plaintext]\n";
    dot << "    fileReport [label=<\n";
    dot << "        <table border=\"1\" cellborder=\"1\">\n";
    dot << "            <tr><td><b>REPORTE FILE</b></td></tr>\n";
    dot << "            <tr><td>Ruta: " << htmlEscape(ruta) << "</td></tr>\n";
    dot << "            <tr><td>" << htmlEscape(displayContent) << "</td></tr>\n";
    dot << "        </table>\n";
    dot << "    >];\n";
    dot << "}\n";
    return writeDotAndExport(dot.str(), outputPath, outError);
}

/** Genera reporte TREE: grafo de inodos y bloques desde inodo 0. */
static bool generateTreeReport(const std::string& diskPath, const std::string& partName, const std::string& outputPath, std::string& outError) {
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        outError = "No se pudo obtener la partición.";
        return false;
    }
    Superblock sb;
    if (!manager::readSuperblock(diskPath, part_start, sb, outError)) {
        if (outError.empty()) outError = "No se puede leer el superbloque.";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "s_magic inválido (no es partición EXT2).";
        return false;
    }

    std::set<int> visited;
    std::set<int> blocksEmitted;
    std::queue<int> toVisit;
    toVisit.push(0);

    std::ostringstream nodesOs;
    std::ostringstream edgesOs;

    while (!toVisit.empty()) {
        int inodeIdx = toVisit.front();
        toVisit.pop();
        if (visited.count(inodeIdx)) continue;
        visited.insert(inodeIdx);

        Inode ino;
        if (!manager::readInode(diskPath, part_start, sb, inodeIdx, ino, outError)) {
            if (outError.empty()) outError = "No se puede leer el inodo.";
            return false;
        }

        nodesOs << "    inode" << inodeIdx << " [label=<\n";
        nodesOs << "        <table border=\"1\" cellborder=\"1\">\n";
        nodesOs << "            <tr><td colspan=\"2\"><b>INODO " << inodeIdx << "</b></td></tr>\n";
        nodesOs << "            <tr><td>i_uid</td><td>" << ino.i_uid << "</td></tr>\n";
        nodesOs << "            <tr><td>i_gid</td><td>" << ino.i_gid << "</td></tr>\n";
        nodesOs << "            <tr><td>i_s</td><td>" << ino.i_s << "</td></tr>\n";
        nodesOs << "            <tr><td>i_atime</td><td>" << htmlEscape(formatDateFromInt(ino.i_atime)) << "</td></tr>\n";
        nodesOs << "            <tr><td>i_ctime</td><td>" << htmlEscape(formatDateFromInt(ino.i_ctime)) << "</td></tr>\n";
        nodesOs << "            <tr><td>i_mtime</td><td>" << htmlEscape(formatDateFromInt(ino.i_mtime)) << "</td></tr>\n";
        nodesOs << "            <tr><td>i_type</td><td>" << (ino.i_type == 0 ? "0" : "1") << "</td></tr>\n";
        nodesOs << "            <tr><td>i_perm</td><td>" << htmlEscape(permToString(ino.i_perm)) << "</td></tr>\n";
        for (int i = 0; i < 15; ++i)
            nodesOs << "            <tr><td>i_block[" << i << "]</td><td>" << ino.i_block[i] << "</td></tr>\n";
        nodesOs << "        </table>\n";
        nodesOs << "    >];\n";

        for (int i = 0; i < 15; ++i) {
            if (ino.i_block[i] == -1) continue;
            int blockIdx = ino.i_block[i];
            edgesOs << "    inode" << inodeIdx << " -> block" << blockIdx << ";\n";

            if (blocksEmitted.count(blockIdx)) continue;
            blocksEmitted.insert(blockIdx);

            if (ino.i_type == 0) {
                FolderBlock fb;
                if (!manager::readFolderBlock(diskPath, part_start, sb, blockIdx, fb, outError)) {
                    if (outError.empty()) outError = "No se puede leer el bloque.";
                    return false;
                }
                nodesOs << "    block" << blockIdx << " [label=<\n";
                nodesOs << "        <table border=\"1\" cellborder=\"1\">\n";
                nodesOs << "            <tr><td colspan=\"2\"><b>BLOCK " << blockIdx << "</b></td></tr>\n";
                nodesOs << "            <tr><td>name</td><td>inode</td></tr>\n";
                for (int e = 0; e < 4; ++e) {
                    std::string name = charArrayToString(fb.b_content[e].b_name, 12);
                    int binodo = fb.b_content[e].b_inodo;
                    nodesOs << "            <tr><td>" << htmlEscape(name) << "</td><td>" << binodo << "</td></tr>\n";
                    if (binodo != -1) {
                        edgesOs << "    block" << blockIdx << " -> inode" << binodo << ";\n";
                        if (name != "." && name != ".." && visited.count(binodo) == 0)
                            toVisit.push(binodo);
                    }
                }
                nodesOs << "        </table>\n";
                nodesOs << "    >];\n";
            } else {
                FileBlock fblk;
                if (!manager::readFileBlock(diskPath, part_start, sb, blockIdx, fblk, outError)) {
                    if (outError.empty()) outError = "No se puede leer el bloque.";
                    return false;
                }
                std::string content = fileBlockContentToString(fblk.b_content, 64);
                nodesOs << "    block" << blockIdx << " [label=<\n";
                nodesOs << "        <table border=\"1\" cellborder=\"1\">\n";
                nodesOs << "            <tr><td><b>BLOCK " << blockIdx << "</b></td></tr>\n";
                nodesOs << "            <tr><td>" << htmlEscape(content) << "</td></tr>\n";
                nodesOs << "        </table>\n";
                nodesOs << "    >];\n";
            }
        }
    }

    std::ostringstream dot;
    dot << "digraph G {\n";
    dot << "    rankdir=LR;\n";
    dot << "    node [shape=plaintext];\n";
    dot << nodesOs.str();
    dot << edgesOs.str();
    dot << "}\n";

    std::filesystem::path out(outputPath);
    std::filesystem::path parent = out.parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            outError = "No se pudo crear el directorio de salida.";
            return false;
        }
    }

    std::filesystem::path dotPath = parent.empty() ? std::filesystem::path(out.stem().string() + ".dot") : parent / (out.stem().string() + ".dot");
    {
        std::ofstream dotFile(dotPath.string());
        if (!dotFile.is_open()) {
            outError = "No se pudo crear el archivo DOT.";
            return false;
        }
        dotFile << dot.str();
    }

    std::string ext = getExtension(outputPath);
    std::string format = "png";
    if (ext == ".jpg" || ext == ".jpeg") format = "jpg";
    else if (ext == ".pdf") format = "pdf";

    std::string cmd = "dot -T" + format + " \"" + dotPath.string() + "\" -o \"" + outputPath + "\"";
    if (std::system(cmd.c_str()) != 0) {
        outError = "Error al ejecutar Graphviz (dot). Verifique que esté instalado.";
        return false;
    }
    return true;
}

std::string runRep(const std::string& name, const std::string& path, const std::string& id,
                   const std::string& ruta) {
    if (id.empty()) return "Error: se requiere -id.";
    std::string diskPath, partName;
    if (!manager::getMountById(id, diskPath, partName)) {
        return "Error: el id no está montado.";
    }
    if (path.empty()) return "Error: se requiere -path.";

    std::string nameLower = name;
    for (char& c : nameLower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (nameLower == "file") {
        if (ruta.empty()) return "Error: se requiere -ruta para el reporte file.";
        std::string err;
        if (!generateFileReport(diskPath, partName, path, ruta, err))
            return "Error: " + err;
        return "Reporte File generado: " + path;
    }
    if (nameLower == "disk") {
        std::string err;
        if (!generateDiskReport(diskPath, path, err))
            return "Error: " + err;
        return "Reporte Disk generado: " + path;
    }
    if (nameLower == "inode") {
        if (ruta.empty()) return "Error: se requiere -ruta para el reporte inode.";
        std::string err;
        if (!generateInodeReport(diskPath, partName, path, ruta, err))
            return "Error: " + err;
        return "Reporte Inode generado: " + path;
    }
    if (nameLower == "block") {
        if (ruta.empty()) return "Error: se requiere -ruta para el reporte block.";
        std::string err;
        if (!generateBlockReport(diskPath, partName, path, ruta, err))
            return "Error: " + err;
        return "Reporte Block generado: " + path;
    }
    if (nameLower == "ls") {
        if (ruta.empty()) return "Error: se requiere -ruta para el reporte ls.";
        std::string err;
        if (!generateLsReport(diskPath, partName, path, ruta, err))
            return "Error: " + err;
        return "Reporte Ls generado: " + path;
    }
    if (nameLower == "mbr") {
        std::string err;
        if (!generateMbrReport(diskPath, path, err))
            return "Error: " + err;
        return "Reporte MBR generado: " + path;
    }
    if (nameLower == "sb") {
        std::string err;
        if (!generateSbReport(diskPath, partName, path, err))
            return "Error: " + err;
        return "Reporte Superblock generado: " + path;
    }
    if (nameLower == "bm_inode") {
        std::string err;
        if (!generateBmInodeReport(diskPath, partName, path, err))
            return "Error: " + err;
        return "Reporte Bitmap Inode generado: " + path;
    }
    if (nameLower == "bm_block") {
        std::string err;
        if (!generateBmBlockReport(diskPath, partName, path, err))
            return "Error: " + err;
        return "Reporte Bitmap Block generado: " + path;
    }
    if (nameLower == "tree") {
        std::string err;
        if (!generateTreeReport(diskPath, partName, path, err))
            return "Error: " + err;
        return "Reporte Tree generado: " + path;
    }

    return "Error: reporte no reconocido (name=" + name + ").";
}


} // namespace commands
} // namespace extreamfs
