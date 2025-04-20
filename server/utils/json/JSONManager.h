#ifndef JSONMANAGER_H
#define JSONMANAGER_H



class JSONManager
{

    public:

        static void create_generic_file(const char* file_path);
        static void append_data_to_file(const char* file_path, const char* data);

};



#endif //JSONMANAGER_H
