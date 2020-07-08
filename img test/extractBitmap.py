### import libs ###
import numpy as np
from PIL import Image # load images
import sys, getopt # for arguments
import math

def weightedGrayscaleConversion(img, weights = [0.2989, 0.5870, 0.1140]):
    weightsTiles = np.tile(weights, reps=(img.shape[0], img.shape[1], 1))
    return np.sum(weightsTiles * img, axis=2)

def threshold(img, rate = 0.5):
    return np.array([[255 if val > 255 * rate else 0 for val in line] for line in img])

def BayerMatrix(n):
    return 255 * (1 + BayerSubMatrix(n)) / (1 + (n * n))

def BayerSubMatrix(n):
    if n <= 2:
        return np.array([[0, 2], [3, 1]])
    else:
        recurse = BayerSubMatrix(n >> 1)
        return np.bmat([[4 * recurse,     4 * recurse + 2],
                        [4 * recurse + 3, 4 * recurse + 1]])

def bayerDithering(img, n = 8):
    bayerM = BayerMatrix(n)
    rows, cols = img.shape
    result = np.zeros(img.shape)
    for i in range(rows):
        for j in range(cols):
            if img[i, j] >  bayerM[i & (n-1), j & (n-1)] :
                result[i, j] = 255
            else:
                result[i, j] = 0
    return result

def main(argv):

   # variables
   inputImg = ''
   outputfile = ''
   outputImg = ''
   verbose = False

   # parseArgument
   try:
      opts, args = getopt.getopt(argv,"hvi:o:",["help", "verbose", "inputImg=","outputfile=", "outputimg="])
   except getopt.GetoptError:
      print ('extractBitmap.py -i <input image> -o <output file> --outputimg <optional output image file> ')
      sys.exit(2)
   for opt, arg in opts:
      if opt in ("-h", "--help"):
         print ('extractBitmap.py -i <input image> -o <output file> --outputimg <optional output image file> ')
         sys.exit()
      elif opt in ("-v", "--verbose"):
         verbose = True
      elif opt in ("-i", "--inputImg"):
         inputImg = arg
      elif opt in ("-o", "--outputfile"):
         outputfile = arg
      elif opt in ("--outputimg"):
         outputImg = arg

   if verbose:
      print ('Input file is ', inputImg, " split :", inputImg.split("."))
      print ('Output file is ', outputfile, " split :", outputfile.split("."))

   inputImgName, inputImgExt = inputImg.split(".")
   outputFileName, outputFileExt = outputfile.split(".")
   imgRead = Image.open(inputImg)

   maxHeight = 384
   newWidth = math.floor(imgRead.width * maxHeight / imgRead.height)
   # resize
   imgResized = imgRead.resize((newWidth, maxHeight), Image.LANCZOS) #Image.NEAREST, Image.BILINEAR, Image.BICUBIC, Image.LANCZOS, Image.ANTIALIAS
   imgResized.save(inputImgName + 'resized.' + inputImgExt)  # save resized img

   f = open(outputfile, "w")

   f.write("#ifndef _%s_h_ \n" % outputFileName)
   f.write("#define _%s_h_ \n" % outputFileName)
   f.write('\n')

   f.write("#define %s_width %d \n" % (outputFileName, imgResized.height))
   f.write("#define %s_height %d \n" % (outputFileName, imgResized.width))

   f.write("static const uint8_t PROGMEM %s_data[] = {\n" % outputFileName)
   
   img = np.array(imgResized) # numpy convesion
   img = weightedGrayscaleConversion(img) # grayScale conversion
   
   # img = threshold(img, 0.5) # apply threshold
   img = bayerDithering(img, 16)# apply bayer Dithering

   img = np.where(img > 0, 0, 1)

   saveImg = Image.fromarray(np.uint8((1-img)*255))
   # save a image using extension 
   if outputImg != '': 
      saveImg.save(outputImg) 

   rows, cols = img.shape
   for c in range(cols):
      col = img[:, -c]
      subGroup = [col[n:n+8] for n in range(0, len(col), 8)]
      subString = [ "".join([str(x) for x in group]) for group in subGroup]
      subInt = [int(x, 2) for x in subString]
      hexStr = [hex(x) for x in subInt]

      if verbose:
         print(col)
         print(subGroup)
         print(subString)
         print(subInt)
         print(hexStr)

      for h in hexStr:
         f.write("%s," % h)
      f.write("\n")

   f.write("};\n")
   f.write("\n")
   f.write("#endif // _%s_h_" % outputFileName)
   f.close()

if __name__ == "__main__":
   main(sys.argv[1:])