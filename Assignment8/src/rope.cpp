#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        for (int i = 0; i < num_nodes; ++i) {
            //double px = start.x + i * (end.x - start.x) / (num_nodes - 1);
            //double py = start.y + i * (end.y - start.y) / (num_nodes - 1);
            Vector2D position = start + (double) i * (end - start) / (double)(num_nodes - 1);
            Mass* tempMass = new Mass(position, node_mass, false);
            tempMass->last_position = position;
            masses.push_back(tempMass);
            // 构造弹簧
            if (i > 0) {
                Spring* tempSpring = new Spring(masses[i-1], masses[i], k);
                springs.push_back(tempSpring);
            }

        }
//        Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            double len = (s->m2->position - s->m1->position).norm();
            Vector2D f = - (s->k) * (s->m2->position - s->m1->position) / len * (len - s->rest_length);
            s->m1->forces -= f;
            s->m2->forces += f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                Vector2D a = m->forces / m->mass + gravity - 0.005 * m->velocity / m->mass; // 加速度
                m->velocity += a * delta_t;
                m->position += m->velocity * delta_t;
                // TODO (Part 2): Add global damping

            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            double len = (s->m2->position - s->m1->position).norm();
            Vector2D f = -s->k * (s->m2->position - s->m1->position) / len * (len - s->rest_length);
            s->m1->forces -= f;
            s->m2->forces += f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                Vector2D a = m->forces / m->mass + gravity; // 加速度
                //m->position = temp_position + (temp_position - m->last_position) + a * delta_t * delta_t;
                m->position = temp_position + (1 - 0.00005) * (temp_position - m->last_position) + a * delta_t * delta_t;
                m->last_position = temp_position;
                // TODO (Part 4): Add global Verlet damping

            }
            m->forces = Vector2D(0, 0);
        }
    }
}
