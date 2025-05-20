#include <cstring>
#include <string>
#include <vector>

// Met à jour la valeur d'une clé dans une section donnée d'un fichier INI
void update_ini_value(const char* ini_path, const std::string& section, const std::string& key, const std::string& new_value) {
    FILE* file = fopen(ini_path, "r");
    if (!file) return;

    std::vector<std::string> lines;
    char buffer[1024];
    bool in_section = false;
    bool key_found = false;

    while (fgets(buffer, sizeof(buffer), file)) {
        std::string line(buffer);

        // Détecter la section
        if (line.find("[") == 0 && line.find("]") != std::string::npos) {
            // Si la clé n'a pas été trouvée dans la section précédente, on l'ajoute avant de passer à la nouvelle section
            if (in_section && !key_found) {
                lines.push_back(key + "=" + new_value + "\n");
                key_found = true; // Pour éviter de l'ajouter plusieurs fois
            }
            std::string current_section = line.substr(1, line.find("]") - 1);
            in_section = (current_section == section);
        } else if (in_section && line.find(key + "=") == 0) {
            // Modifier la ligne de la clé
            line = key + "=" + new_value + "\n";
            key_found = true;
        }

        lines.push_back(line);
    }
    fclose(file);

    // Si la section existe mais que la clé n'a jamais été trouvée, on l'ajoute à la fin de la section
    if (in_section && !key_found) {
        lines.push_back(key + "=" + new_value + "\n");
    }

    // Réécrire le fichier
    file = fopen(ini_path, "w");
    if (!file) return;
    for (const auto& l : lines) {
        fputs(l.c_str(), file);
    }
    fclose(file);
}
