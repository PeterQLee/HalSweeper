import tensorflow as tf
import numpy
import matplotlib.pyplot as plt
from PIL import Image, ImageSequence
import sys
import time
sys.path.insert(0,'../build/lib.linux-x86_64-3.5/')
import BoardPy


def weight_variable(shape):
  initial = tf.truncated_normal(shape, stddev=0.05)
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
        self.epochs=500
        self.end=False
        self.convolutions=[]
        self.full_conn=[]
        self.board=BoardPy.BoardPy(*DIMS)
        self.placeh_out=numpy.zeros((1,DIMS[0]*DIMS[1]))

        self.reward=tf.placeholder(tf.float32,shape[None,DIMS[0],DIMS[1]])
        self.sess=tf.InteractiveSession()
        self.input_vec=tf.placeholder(tf.float32, shape=[None,DIMS[0]*32,32*DIMS[1],4])
        #self.x_image=tf.reshape(self.input_vec,[-1,DIMS[0],DIMS[1],4])
                                      
        self.out_next=tf.placeholder(tf.float32,shape=[None,DIMS[0]*DIMS[1]])

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
        self.out_vec=tf.nn.softmax(self.full_conn[-1][0])

        #Training stuff
        self.error=tf.reduce_mean(tf.square(tf.add(self.reward,tf.sub(self.out_next, self.out_vec))))#tf.nn.softmax_cross_entropy_with_logits(self.out_vec,self.out_next))
        #self.train_step=tf.train.GradientDescentOptimizer(0.1).minimize(self.error)
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
        reward={1:-.1,2:-.5,0:0.05}
        #ret=self.board.click(unravel_index(out_vec.argmax(),out_vec.shape))
        v=out_vec.argmax()
        #print(out_vec.shape)
        #if (abs(out_vec[0][v])<=0.001):
        #print (out_vec)
        #print (v,out_vec[0][v])
        ret_vec=numpy.zeros(out_vec.shape)
        ret=self.board.click(v//self.DIMS[0],v%self.DIMS[1])
        
        self.click_filter[v]=0
        #print (ret)
        if ret==2: #Game end
          #print ('f to pay respecks')
          self.end=True
        ret_vec[v]=reward[ret]
        return ret_vec
        #return max(reward[ret],-out_vec[0][out_vec.argmax()])

    #def feed_forward(self):
    #    return self.sess.run(self.out_vec,feed_dict={self.in:self.board.imgboard,)
    
    # def train(self):
    #     image=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
    #     image=(numpy.array(image,dtype=numpy.float32))/255.0 #del -127

    #     guess_out,weights,=self.sess.run([self.out_vec,self.convolutions[0][0]],feed_dict={self.input_vec:image,self.out_next:self.placeh_out})
    #     #print(weights)
    #     #guess_out=numpy.multiply(guess_out,self.click_filter)
    #     rew=self.perform_click(guess_out)
    #     #guess_out.item(unravel_index(out_vec.argmax(),out_vec.shape))+=rew
    #     guess_out[0][guess_out.argmax()]+=rew

    #     self.sess.run([self.train_step],feed_dict={self.input_vec:image,self.out_next:guess_out})
        
    def evaluate(self):

      while not self.end:
        image=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
        image=(numpy.array(image,dtype=numpy.float32)-127.0)/255.0
        
        guess_out,=self.sess.run([self.out_vec],feed_dict={self.input_vec:image,self.out_next:self.placeh_out,self.reward:0.0})
        guess_out=numpy.multiply(guess_out,self.click_filter)
        reward=self.perform_click(guess_out)
        self.Q_record.append((guess_out,guess_out.argmax()))
        self.reward.append(reward)
        self.frames.append(image)
        #img=Image.fromarray(self.board.imgboard.view(numpy.uint8).reshape(10*32,-1,4))
        #img.show()
        
    def train(self):
      if len(self.Q_record)==0:return
      DISCOUNT=0.5
      currQ=self.Q_record[-1][0]
      currreward=self.reward[-1]
      currimg=self.frames[-1]
      v=self.Q_record[-1][1]
      currQ[0][v]+=currreward
      lastQ=currQ[0][v]
      self.sess.run([self.train_step],feed_dict={self.input_vec:currimg,self.out_next:currQ})
      for q in range(len(self.Q_record)-2,-1,-1):

        currQ=self.Q_record[q][0]
        currreward=self.reward[q]
        currimg=self.frames[q]
        v=self.Q_record[q][1]
        currQ[0][v]+=currreward+DISCOUNT*lastQ
        lastQ=currQ[0][v]
        self.sess.run([self.train_step],feed_dict={self.input_vec:currimg,self.out_next:currQ})

        
    def eval(self):
        scores=[]
        # self.board.click(0,0)
        # image=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
        # image=(numpy.array(image,dtype=numpy.float32))/255.0 #del -127
        # guess_out,weights,=self.sess.run([self.out_vec,self.convolutions[0][0]],feed_dict={self.input_vec:image,self.out_next:self.placeh_out})
        # print(weights)
        # img=Image.fromarray(self.board.imgboard.view(numpy.uint8).reshape(10*32,-1,4))
        # img.show()
        
        for i in range(self.epochs):
          self.Q_record=[]
          self.frames=[]
          self.reward=[]
          self.click_filter=numpy.full(10*10,1.0)
          if (i%100==0):
            print('epoch',i)
          self.end=False
          self.board.click(5,5)
          while not self.end:
            self.evaluate()
            #img=Image.fromarray(self.board.imgboard.view(numpy.uint8).reshape(10*32,-1,4))
            #img.show()
            #time.sleep(2)
          self.train()
          scores.append(self.board.score())
          self.board.remake(10,10)
         
        self.end=False
        plt.plot(numpy.arange(0,len(scores)),scores)
        plt.savefig('progress.png')
        self.board.click(5,5)
        while not self.end:
          image=self.board.imgboard.view(numpy.uint8).reshape(1,self.DIMS[0]*32,-1,4)
          image=(numpy.array(image,dtype=numpy.float32)-127.0)/255.0

          guess_out,=self.sess.run([self.out_vec],feed_dict={self.input_vec:image,self.out_next:self.placeh_out})
          guess_out=numpy.multiply(guess_out,self.click_filter)
          reward=self.perform_click(guess_out)
          self.Q_record.append((guess_out,guess_out.argmax()))
          self.reward.append(reward)
          self.frames.append(image)
          img=Image.fromarray(self.board.imgboard.view(numpy.uint8).reshape(10*32,-1,4))
          img.show()
if __name__=='__main__':
  h=Hal((10,10))
  h.eval()
