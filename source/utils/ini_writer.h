#ifndef INI_WRITER_H
#define INI_WRITER_H

#include <string>

// Met à jour la valeur d'une clé dans une section donnée d'un fichier INI.
// ini_path : chemin du fichier INI
// section  : nom de la section (ex: "Dropbox")
// key      : nom de la clé (ex: "RefreshToken")
// new_value: nouvelle valeur à écrire
void update_ini_value(const char* ini_path,
                     const std::string& section,
                     const std::string& key,
                     const std::string& new_value);

#endif // INI_WRITER_H