/*
  * Copyright (c) 2018 Heng Yuan
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
 #ifndef TUTORIAL_PATH_H
 #define TUTORIAL_PATH_H
 
 #include <agg_basics.h>
 #include <agg_trans_affine.h>
 
 class SimplePath
 {
 public:
     SimplePath(const double* points, int size)
     : m_points (points),
       m_size (size),
       m_pos (0)
     {
     }
 
     void rewind (int path = 0)
     {
         m_pos = 0;
     }
 
     unsigned vertex(double* x, double* y)
     {
         if (m_pos < m_size)
         {
             *x = m_points[m_pos++];
             *y = m_points[m_pos++];
             return (m_pos == 2 ? agg::path_cmd_move_to : agg::path_cmd_line_to);
         }
         return agg::path_cmd_stop;
     }
 
 private:
     const double *m_points;
     int           m_size;
     int           m_pos;
 };
 
 struct CmdVertex
 {
     agg::path_commands_e    cmd;
     double  x;
     double  y;
 };
 
 class CmdVertexPath
 {
 public:
     CmdVertexPath(const CmdVertex* vertices, int size)
     : m_vertices (vertices),
       m_size (size),
       m_pos (0)
     {
     }
 
     void rewind (int path = 0)
     {
         m_pos = 0;
     }
 
     unsigned vertex(double* x, double* y)
     {
         if (m_pos < m_size)
         {
             *x = m_vertices[m_pos].x;
             *y = m_vertices[m_pos].y;
             agg::path_commands_e cmd = m_vertices[m_pos].cmd;
 
             ++m_pos;
             return cmd;
         }
         return agg::path_cmd_stop;
     }
 
     int size () const { return m_size; }
 
 private:
     const CmdVertex *m_vertices;
     int m_size;
     int m_pos;
 };
 
 #endif  /* TUTORIAL_PATH_H */
