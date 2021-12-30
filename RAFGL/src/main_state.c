#include <main_state.h>
#include <glad/glad.h>
#include <math.h>

#include <rafgl.h>


struct meteor {
  int x;
  int y;
};

static rafgl_raster_t raster, player, space, gradient_meteor;
static rafgl_texture_t texture;

static int raster_width = 0, raster_height = 0;
int meteor_count = 7;
struct meteor meteors[3];

void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    for(int i = 0; i < meteor_count; i++)
    {
        meteors[i].x = rand()%350;
        meteors[i].y = 0 - i * 250;
    }

    rafgl_raster_load_from_image(&player, "res/images/player.png");
    rafgl_raster_load_from_image(&space, "res/images/space.png");
    raster_width = 350;
    raster_height = 700;

    rafgl_raster_init(&raster, raster_width, raster_height);
    rafgl_raster_init(&gradient_meteor, 50, 50);

    for(int i = 0; i < raster_width; i++)
    {
        for(int j = 0; j < raster_height; j++)
        {
            rafgl_pixel_rgb_t sampled = pixel_at_m(space, i, j);
            pixel_at_m(raster, i, j) = sampled;
        }
    }

    rafgl_texture_init(&texture);
}

int za_warudo = 0;
float selector = 0;
int velocityX = 0;
int meteor_velocity = 5;
int playerX = 175, playerY = 600;

void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{
    rafgl_pixel_rgb_t sampled, temp;

    if(za_warudo != 0)
        za_warudo++;

    if(za_warudo>100)
        za_warudo=-100;

    if(game_data->keys_down[RAFGL_KEY_SPACE] && za_warudo == 0)
    {
        za_warudo=5;
    }

    for(int i = 0; i < raster_width; i++)
    {
        temp = pixel_at_m(space, i, space.height-1);
        for(int j = raster_height-1; j > 0; j--)
        {
            sampled = pixel_at_m(space, i, j-1);
            pixel_at_m(space, i, j) = sampled;
            pixel_at_m(raster, i, j) = sampled;
        }
        pixel_at_m(space, i, 0) = temp;
    }

    if(game_data->keys_down[RAFGL_KEY_RIGHT] && velocityX < 10)
    {
        velocityX+=2;
    }
    if(game_data->keys_down[RAFGL_KEY_LEFT] && velocityX > -10)
    {
        velocityX-=2;
    }

    velocityX += velocityX > 0 ? -1 : velocityX < 0 ? 1 : 0;

    playerX += velocityX;


    for(int k = 0; k < meteor_count; k++)
    {
        meteors[k].y+=rafgl_clampi(meteor_velocity-za_warudo*meteor_velocity/30, 0, meteor_velocity);

        if (meteors[k].y > 750)
        {
            meteors[k].y = -100;
            meteors[k].x = rand() % 350;
            //meteor_velocity += rafgl_clampi(rand()%2, 0 , 1);  Ovo treba da ubrzava meteore, mada previse ih ubrza, pa sam odustao
        }

        int light = 0;
        for(int n = 0; n < gradient_meteor.width; n++)
        {
            for(int m = 0; m < gradient_meteor.height; m++)
            {
                if(rafgl_distance2D(m, n, 25, 25) > 25)
                {
                    pixel_at_m(gradient_meteor, m, n).rgba = rafgl_RGB(0, 0, 0);
                }
                else
                {
                    light = rafgl_abs_m(((int)(rafgl_distance2D(m, n, 25, 25) * 2 + meteors[k].y) % 120) - 60);
                    pixel_at_m(gradient_meteor, m, n).rgba = rafgl_RGB(200, light*2, light*2);
                }
            }
        }

        draw_sprite(gradient_meteor, meteors[k].x, meteors[k].y, 0);
    }

    draw_sprite(player, playerX, playerY, 0);

    if(velocityX < 0)
        for(int n = 0; n > velocityX; n--)
    {
        draw_sprite(player, playerX-n*3, playerY, 30+20*rafgl_abs_m(velocityX));
    }
    else
        for(int n = 0; n < velocityX; n++)
    {
        draw_sprite(player, playerX-n*3, playerY, 30+20*rafgl_abs_m(velocityX));
    }

    for(int i = 0; i < raster_width; i++)
    {
        for(int j = 0; j < raster_height; j++)
        {
            if(rafgl_distance2D(i, j, 175, 800) < za_warudo*10)
            {
                sampled = pixel_at_m(raster, i, j);
                sampled.r = 255 - sampled.r;
                sampled.g = 255 - sampled.g;
                sampled.b = 255 - sampled.b;
                pixel_at_m(raster, i, j) = sampled;
            }
        }
    }

    rafgl_texture_load_from_raster(&texture, &raster);
}


void main_state_render(GLFWwindow *window, void *args)
{
    rafgl_texture_show(&texture, 0);
}


void main_state_cleanup(GLFWwindow *window, void *args)
{
    rafgl_raster_cleanup(&raster);
    rafgl_texture_cleanup(&texture);
}

void draw_sprite(rafgl_raster_t sprite, int x, int y, int transparency)
{
    rafgl_pixel_rgb_t sampled, result, origin;
    int sprite_width = sprite.width, sprite_height = sprite.height, ioffset = 0, joffset = 0;
    if (sprite.width/2 + x > raster.width)
    {
        sprite_width -= sprite.width/2 + x - raster.width;
    }
    else if (x - sprite.width/2 < 0)
    {
        ioffset += sprite.width/2 - x;
    }

    if (sprite.height/2 + y > raster.height)
    {
        sprite_height -= sprite.height/2 + y - raster.height;
    }
    else if (y - sprite.height/2 < 0)
    {
        joffset += sprite.height/2 - y;
    }

    for(int i = ioffset; i < sprite_width; i++)
    {
        for(int j = joffset; j < sprite_height; j++)
        {
            if(pixel_at_m(sprite, i, j).rgba != rafgl_RGB(0, 0, 0))
            {
                sampled = pixel_at_m(sprite, i, j);
                result = sampled;
                origin = pixel_at_m(raster, x-(sprite.width/2)+i, y-(sprite.height/2)+j);
                result.r = sampled.r * (1.0f * (255 - transparency) / 255) + origin.r * (1.0f * transparency / 255);
                result.g = sampled.g * (1.0f * (255 - transparency) / 255) + origin.g * (1.0f * transparency / 255);
                result.b = sampled.b * (1.0f * (255 - transparency) / 255) + origin.b * (1.0f * transparency / 255);
                pixel_at_m(raster, x-(sprite.width/2)+i, y-(sprite.height/2)+j) = result;
            }
        }
    }
}
