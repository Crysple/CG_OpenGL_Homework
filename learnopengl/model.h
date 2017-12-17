//
//  model.h
//  testOpenGL
//
//  Created by Eser on 26/10/2017.
//  Copyright © 2017 Crysple. All rights reserved.
//

#ifndef model_h
#define model_h

//glew
#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb-master/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#define WAIT 0
#define TRIANGLE 1
#define RECTANGLE 2
using namespace std;

unsigned int TextureFromFile(const char *path);

class Model
{
public:
    /*  Model Data */
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    bool gammaCorrection;
    
    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    Model(string const &path,const char *npath, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path,npath);
    }
    
    // draws the model, and thus all its meshes
    void Draw(Shader shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
private:
    void calcu(vector<Vertex>& original_vertices, vector<Face>& original_faces, vector<glm::vec2> texCoords){
        vector<Vertex> final_vertices;
        auto bindNormal = [&](unsigned int face_idx){
            auto face_x = original_faces[face_idx];
            auto face_y = original_faces[face_idx+1];
            auto face_z = original_faces[face_idx+2];
            Vertex& x = original_vertices[face_x.pos_id];
            Vertex& y = original_vertices[face_y.pos_id];
            Vertex& z = original_vertices[face_z.pos_id];
            glm::vec3 vt1 = x.Position - y.Position;
            glm::vec3 vt2 = z.Position - y.Position;
            glm::vec3 normal = glm::cross(vt1,vt2);
            x.Normal = y.Normal = z.Normal = normal;
        };
        auto bindTexture = [&](unsigned int face_idx){
            for(int i=0;i<3;++i){
                auto now_idx = face_idx+i;
                auto vertex_id = original_faces[now_idx].pos_id;
                auto tex_id = original_faces[now_idx].tex_id;
                original_vertices[vertex_id].TexCoords = texCoords[tex_id];
            }
        };
        auto addNew = [&](unsigned int face_idx){
            for(int i=0;i<3;++i){
                auto now_idx = face_idx+i;
                auto vertex_id = original_faces[now_idx].pos_id;
                final_vertices.push_back(original_vertices[vertex_id]);
            }
        };
        for(int i=0;i<original_faces.size();i+=3){
            bindNormal(i);
            bindTexture(i);
            addNew(i);
        }
        original_vertices = final_vertices;
    }
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path,const char *npath)
    {
        vector<Vertex> vertices;
        vector<Face> faces;
        vector<Texture> textures;
        
        vector<glm::vec3> position;
        vector<glm::vec2> texCoords;
        // 1. retrieve the vertex/fragment source code from filePath
        std::stringstream objStream;
        std::ifstream objFile;
        // ensure ifstream objects can throw exceptions:
        objFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            objFile.open(path);
            // read file's buffer contents into streams
            objStream << objFile.rdbuf();
            //std::cout<<objStream.str()<<std::endl;
            // close file handlers
            objFile.close();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::OBJ::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        string prefix;
        int status=WAIT;
        bool need_new = true;
        auto rectangle2triangle = [&](Face i_face){
            auto fir = faces[faces.size()-3],sec = faces[faces.size()-1];
            faces.push_back(fir);
            faces.push_back(sec);
            faces.push_back(i_face);
        };
        while(true){
            if(need_new){
                if(!(objStream >> prefix))
                    break;
            }
            else need_new = true;
            if(prefix.length()>2) continue;
            switch (prefix[0]) {
                case 'v' : {
                    switch (prefix[1]) {
                        case 't' : {
                            glm::vec2 vector2;
                            objStream >>vector2.x >>vector2.y;
                            texCoords.push_back(vector2);
                            break;
                        } // texcoords
                        case 'n' : {} break; // normals
                        default : { // points
                            glm::vec3 vector3;
                            objStream >> vector3.x >> vector3.y >> vector3.z;
                            position.push_back(vector3);
                            // put in a mesh etc
                        }
                    }
                    break;
                }
                case 'f' : {
                    char temp_char;
                    Face i_face;
                    for(int i=0;i<(status==RECTANGLE?4:3);++i){
                        objStream>>i_face.pos_id>>temp_char>>i_face.tex_id;
                        --i_face.pos_id;
                        --i_face.tex_id;
                        //for rectangle
                        if(i==3){
                            rectangle2triangle(i_face);
                        }
                        faces.push_back(i_face);
                    }
                    if(status==WAIT){
                        objStream>>prefix;
                        if(prefix[0]=='f'){
                            status=TRIANGLE;
                            need_new = false;
                        }
                        else{
                            status=RECTANGLE;
                            stringstream temp(prefix);
                            temp>>i_face.pos_id>>temp_char>>i_face.tex_id;
                            --i_face.pos_id;
                            --i_face.tex_id;
                            rectangle2triangle(i_face);
                        }
                    }
                    break;
                }
                default : {} // skip
            }
        }
        
        //process vertice
        for(int i=0;i<position.size();++i){
            Vertex now;
            now.Position = position[i];
            if(i<texCoords.size()){
                now.TexCoords = texCoords[i];
            }
            else now.TexCoords = glm::vec2(0.0f, 0.0f);
            vertices.push_back(now);
        }
        calcu(vertices,faces,texCoords);
        vector<unsigned int> indices;
        for(unsigned int i=0;i<vertices.size();++i) indices.push_back(i);
        //process Texture
        Texture texture;
        //char *npath = "container.jpg";
        texture.id = TextureFromFile(npath);
        texture.type = "texture_diffuse";
        texture.path = string(npath);
        textures.push_back(texture);
        Mesh mesh(vertices,indices,textures);
        meshes.push_back(mesh);
    }
};


unsigned int TextureFromFile(const char *path)
{
    string filename = string(path);
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    
    return textureID;
}


#endif /* model_h */
