from struct import unpack
from gimpfu import *
import os


PALLETE_SIZE = 256
PIXEL_LENGTH = 4

IMAGE_WIDTH = 83
IMAGE_HEIGHT = 85

PLUGIN_MENU_LABEL = 'Loads Heath The unchosen path sprite (.itd)'
PLUGIN_THUMBNAIL_MENU_LABEL = 'Loads a thumbnail for Heath The unchosen path sprit (.itd)'
PLUGIN_MENU_DESCRIPTION = 'Loads sprite images from .itd files'
LOAD_PROC = 'file-hth-itd-load'
LOAD_THUMB_PROC  = 'file-hth-itd-load-thumb'

AUTHOR = 'AlexKimov'
COPYRIGHT = AUTHOR
COPYRIGHT_YEAR = '2024'


class Sprite:
    FileTable = {
       "amulets.itd": (56, 1),
       "axes.itd": (56, 0),
       "bows.itd": (56, 0),
       "elbarms.itd": (48, 0),
       "helmets.itd": (56, 0),
       "hits.itd": (56, 0),
       "panoplys.itd": (56, 0),
       "potions.itd": (56, 1),
       "protectors.itd": (56, 0),
       "quests.itd": (48, 1),
       "rolls.itd": (64, 1),
       "swords.itd": (64, 0)
    } 
        
    @staticmethod
    def load_from_file(file_path):
        """
        Load Sprite from file.
        :param file_path: path to the sprite file
        :return: gimp images
        """

        filename = os.path.basename(file_path)
        fd = open(file_path, 'rb')      
        images = []
        while True:       
            image_data = Sprite._read_next_image(fd, filename)
            if image_data == None: 
                 break  
                 
            image = Sprite._make_image(image_data)            
            images.append(image) 

        return images   
        
    @staticmethod        
    def _read_next_image(fd, fname): 
        try:       
            name = Sprite._read_header(fd, fname)
            palette = Sprite._read_palette(fd) 
            data = Sprite._read_image_data(fd)           
        except: 
            return None
            
        return [name, palette, data]        
            
    @staticmethod
    def _make_image(data): 
        img = gimp.Image(IMAGE_WIDTH, IMAGE_HEIGHT, INDEXED)
        
        img.colormap = data[1]
        #pdb.gimp_message(
        img.filename = data[0].encode()
        layer = gimp.Layer(img, data[0], IMAGE_WIDTH, IMAGE_HEIGHT, INDEXED_IMAGE, 100, NORMAL_MODE)
        rgn = layer.get_pixel_rgn(0, 0, layer.width, layer.height)
        rgn[:, :] = data[2]
        layer.flush()
        
        pdb.gimp_image_insert_layer(img, layer, None, 0)
        
        return img    

    @staticmethod
    def _read_header(fd, fname):      
        skipped_bytes_count = Sprite.FileTable[fname][0] | 0        
        fd.seek(skipped_bytes_count, 1)         
        
        length = unpack('<H', fd.read(2))[0]   
        name = fd.read(length).decode(encoding='Windows-1251')                   
        if Sprite.FileTable[fname][1]:
            count = unpack('<H', fd.read(2))[0]             
            fd.seek(count * 16, 1) 
            
        return name
        
    @staticmethod            
    def _read_palette(fd):
        palette = fd.read(PALLETE_SIZE * PIXEL_LENGTH)     
      
        pal_array = ''
        i = 0
        for x in range(PALLETE_SIZE):
            pal_array += palette[i: i + 3]        
            i = i + 4            

  
        
        return pal_array
        
    @staticmethod        
    def _read_image_data(fd):
        size = unpack('<H', fd.read(2))[0]
        transparent_color_Index = unpack('<B', fd.read(1))[0]   
        data = fd.read(size)
        
        return data
        

def load_itd_thumbnail(file_path, thumb_size):
    img = Sprite.load_from_file(file_path, True)[0]
    width, height = img.width, img.height
    scale = float(thumb_size) / max(width, height)
    if scale and scale != 1.0:
        width = int(width * scale)
        height = int(height * scale)
        pdb.gimp_image_scale(img, width, height)

    return (img, width, height)


def load_itd(file_path, raw_filename):
    try:
        images = Sprite.load_from_file(file_path)
        for img in images[:-1]:
            gimp.Display(img)
            gimp.displays_flush()
        return images[-1]
    except Exception as e:
        fail('Error loading sprite file:\n\n%s!' % e.message)


def register_load_handlers():
    gimp.register_load_handler(LOAD_PROC, 'itd', '')
    pdb.gimp_register_thumbnail_loader(LOAD_PROC, LOAD_THUMB_PROC)


register(
    LOAD_THUMB_PROC,
    PLUGIN_THUMBNAIL_MENU_LABEL,
    '',
    AUTHOR,
    COPYRIGHT,
    COPYRIGHT_YEAR,
    None,
    None,
    [
        (PF_STRING, 'filename', 'The name of the file to load', None),
        (PF_INT, 'thumb-size', 'Preferred thumbnail size', None),
    ],
    [
        (PF_IMAGE, 'image', 'Thumbnail image'),
        (PF_INT, 'image-width', 'Width of full-sized image'),
        (PF_INT, 'image-height', 'Height of full-sized image')
    ],
    load_itd_thumbnail,
    run_mode_param = False
)


register(
    LOAD_PROC,
    PLUGIN_MENU_LABEL,
    '',
    AUTHOR,
    COPYRIGHT,
    COPYRIGHT_YEAR,
    'Heath The unchosen path sprite',
    None,
    [
        (PF_STRING, 'filename', 'The name of the file to load', None),
        (PF_STRING, 'raw-filename', 'The name entered', None),
    ],
    [(PF_IMAGE, 'image', 'Output image')],
    load_itd,
    on_query=register_load_handlers,
    menu='<Load>'
)

main()