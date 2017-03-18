import tensorflow as tf
import numpy
import matplotlib.pyplot as plt
from PIL import Image, ImageSequence
import sys
import time
sys.path.insert(0,'../build/lib.linux-x86_64-3.5/')
import BoardPy
import random


def weight_variable(shape):
  initial = tf.truncated_normal(shape, stddev=0.1)
  return tf.Variable(initial)

def bias_variable(shape):
  initial = tf.constant(0.1, shape=shape)
  return tf.Variable(initial)

def conv2d(x, W,strid):
    #num imgs, x,y,channels
  return tf.nn.conv2d(x, W, strides=strid, padding='SAME')

def meanpool(x,AVG,strid):
  return tf.nn.depthwise_conv2d(x,AVG,strides=strid,padding='VALID')

class Hal:
    def __init__(self,DIMS):
        averagetensor=tf.constant(1.0/32.0/32.0,shape=[32,32,4,1])
        self.DIMS=DIMS
        self.epochs=400
        self.end=False
        self.convolutions=[]
        self.full_conn=[]
        self.board=BoardPy.BoardPy(*DIMS)
        self.placeh_out=numpy.zeros((1,DIMS[0]*DIMS[1]))
        DISCOUNT=tf.constant(0.5)

        self.train_index=tf.placeholder(tf.int32)
        self.sess=tf.InteractiveSession()
        self.input_vec=tf.placeholder(tf.float32, shape=[None,DIMS[0],DIMS[1],1])
        #self.x_image=tf.reshape(self.input_vec,[-1,DIMS[0],DIMS[1],4])
                                      
        #self.out_next=tf.placeholder(tf.float32,shape=[None,DIMS[0]*DIMS[1]])
        self.target=tf.placeholder(tf.float32)

        #Build convolutional layers
        s=self.convolutions
        #self.convolutions.append(self._newconvlayer([32,32,4,11],self.input_vec,[1,32,32,1]))
        print(self.input_vec.get_shape())
        self.convolutions.append([meanpool(self.input_vec,averagetensor,[1,32,32,1]),[-1,10,10,4]]) #Average each 32 block intoo a 10 x 10 matrix
        print(s[-1][0].get_shape())
        self.convolutions.append(self._newconvlayer([3,3,4,100],s[-1][0],[1,1,1,1])) #Try doing a max pool?
        #self.convolutions.append((tf.nn.max_pool(s[-1][0], ksize=[1, 1, 1, 5],
        #                                        strides=[1, 1, 1, 5], padding='SAME'),[]))
        self.convolutions.append(self._newconvlayer([3,3,100,25],s[-1][0],[1,1,1,1]))
        self.convolutions.append(self._newconvlayer([3,3,25,50],s[-1][0],[1,1,1,1]))

        print ((s[0][0]).get_shape())
        print ((s[1][0]).get_shape())
        lastconv=tf.reshape(self.convolutions[-1][0],[-1, DIMS[0]*DIMS[1]*50])

        #build fully connected layers
        f=self.full_conn
        self.full_conn.append(self._newconnectlayer([DIMS[0]*DIMS[1]*50,DIMS[0]*DIMS[1]],lastconv))
        self.out_vec=self.full_conn[-1][0]#tf.nn.softmax(self.full_conn[-1][0])

        #Training stuff
        Q_curr=self.out_vec[0][self.train_index]

        self.error=tf.reduce_mean(tf.square(tf.sub(self.target,Q_curr)))

        self.train_step=tf.train.AdamOptimizer(1e-4).minimize(self.error)
        #tf.global_variables_initializer()
        self.sess.run(tf.initialize_all_variables())
        
    def _newconvlayer(self,dims,last,strides):
        weights=weight_variable(dims)
        bias=bias_variable([dims[-1]])
        h=tf.nn.relu(conv2d(last,weights,strides)+bias)
        return (h,dims,(weights,bias))
      
    def _newconnectlayer(self,dims,last):
        weights=weight_variable(dims)
        bias=bias_variable([dims[-1]])
        h=tf.nn.relu(tf.matmul(last,weights)+bias)
        return (h,dims,(weights,bias))
      
    def perform_click(self, out_vec):
        reward={1:-.1,2:-.1,0:0.05}
        v=out_vec.argmax()
        sample=random.uniform(0,1)
        if sample<0.01:
          x=random.randint(0,9)
          y=random.randint(0,9)
          ret=self.board.click(x,y)
        else:
          ret=self.board.click(v//self.DIMS[0],v%self.DIMS[1])
        
        self.click_filter[v]=0
        #print (ret)
        if ret==2: #Game end
          #print ('f to pay respecks')
          self.end=True

        return reward[ret]
        
    def evaluate(self):

      while not self.end:
        image=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
        image=(numpy.array(image,dtype=numpy.float32)-127.0)/255.0
        
        guess_out,=self.sess.run([self.out_vec],feed_dict={self.input_vec:image,self.target:0,self.train_index:0})
        
        self.rew=self.perform_click(guess_out)
        
    def train(self):
      image=((self.board.mineboard-3.5)/2.8722813232690143) #Normalize mineboard
      image=numpy.pad(image,[1,1],'constant').reshape(-1,self.DIMS[0],self.DIMS[1],1) #reshape to dims
      
      
      zeros=numpy.zeros((1,self.DIMS[0]*self.DIMS[1]))

      guess_out,=self.sess.run([self.out_vec],feed_dict={self.input_vec:image,self.target:0,self.train_index:0})
      guess_out[0]=numpy.multiply(guess_out[0],self.click_filter)
      rew=self.perform_click(guess_out)
      image_1=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
      image_1=(numpy.array(image,dtype=numpy.float32)-127.0)/255.0

      nextQ,=self.sess.run([self.out_vec],feed_dict={self.input_vec:image,self.target:0,self.train_index:0})
      nextQ[0]=numpy.multiply(nextQ[0],self.click_filter)
      
      nextQ=nextQ[0][nextQ.argmax()]
      target=rew+0.5*nextQ
      self.last=self.click_filter.copy()
      self.sess.run([self.train_step],feed_dict={self.input_vec:image,self.target:target,self.train_index:guess_out.argmax()})
        
    def eval(self):
        scores=[]

        for i in range(self.epochs):
          self.rew=None
          self.currQ=None
          
          self.click_filter=numpy.full(10*10,1.0)
          self.last=self.click_filter.copy()
          if (i%10==0):
            print('epoch',i)
          self.end=False
          #self.board.click(5,5)
          #self.click_filter[55]=1.0
          while not self.end:
            self.train()
          #self.train()
          scores.append(self.board.score())
          self.board.remake(10,10)
         
        self.end=False
        plt.plot(numpy.arange(0,len(scores)),scores)
        plt.savefig('progress.png')
        self.board.click(5,5)
        while not self.end:
          image=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
          image=(numpy.array(image,dtype=numpy.float32)-127.0)/255.0

          guess_out,=self.sess.run([self.out_vec],feed_dict={self.input_vec:image,self.target:0,self.train_index:0})
          guess_out=numpy.multiply(guess_out,self.click_filter)
          reward=self.perform_click(guess_out)
          img=Image.fromarray(self.board.imgboard.view(numpy.uint8).reshape(10*32,-1,4))
          img.show()
if __name__=='__main__':
  h=Hal((10,10))
  h.eval()
