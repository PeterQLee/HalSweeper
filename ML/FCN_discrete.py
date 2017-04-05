import tensorflow as tf
import numpy as np
import sys
from PIL import Image, ImageSequence
sys.path.insert(0,'../build/lib.linux-x86_64-3.5/')
import BoardPy
from discboard import discboard

def weight_variable(shape,name,stdv=0.1):
    #return tf.Variable(tf.truncated_normal(shape,stddev=stdv))
    if len(shape)==4:
        return tf.get_variable(name,shape=shape,initializer=tf.contrib.layers.xavier_initializer_conv2d())
    return tf.get_variable(name,shape=shape,initializer=tf.contrib.layers.xavier_initializer())

def bias_variable(shape,val=0.0):
    return tf.Variable(tf.constant(val,shape=shape))


            
    
class FCN:
    def __init__(self):
        kern=[1,1,1,1]

        self.X=tf.placeholder(tf.float32,shape=[None,None,None,None])
        self.target=tf.placeholder(tf.float32,shape=[None,None,None,None])
        self.click_mask=tf.placeholder(tf.float32,shape=[None,None,None,1]) 
        
        W1=weight_variable([5,5,10,32],'W1')
        B1=bias_variable([32])
        H1=tf.nn.relu(tf.nn.conv2d(self.X,W1,kern,'SAME')+B1)

        W2=weight_variable([5,5,32,64],'W2')
        B2=bias_variable([64])
        H2=tf.nn.relu(tf.nn.conv2d(H1,W2,kern,'SAME')+B2)
        
        W3=weight_variable([5,5,64,32],'W3')
        B3=bias_variable([32])
        H3=tf.nn.relu(tf.nn.conv2d(H2,W3,kern,'SAME')+B3)
        
        W4=weight_variable([5,5,32,1],'W4')
        B4=bias_variable([1])
        H4=tf.nn.relu(tf.nn.conv2d(H3,W4,kern,'SAME')+B4)*self.click_mask #Click mask to prevent duplicate clicks

        self.Y=tf.nn.softmax(tf.reshape(H4,[-1]))
        self.Y=tf.reshape(self.Y,tf.shape(self.target))*self.click_mask
        self.sess=tf.InteractiveSession()

        self.error=tf.reduce_mean(tf.square(tf.sub(self.target,self.Y)))
        self.train_step=tf.train.AdamOptimizer(1e-4).minimize(self.error)
        #self.train_step=tf.train.GradientDescentOptimizer(1e-2).minimize(self.error)
        self.sess.run(tf.initialize_all_variables())

                                       
    def predict_prob(self,image,click_mask):
        #image is reused as target because in this case it doesn't matter...
        return self.sess.run(self.Y, feed_dict={self.X:image,self.target:click_mask,self.click_mask:click_mask})

    def update(self,image,click_mask,target):
        self.sess.run(self.train_step,feed_dict={self.X:image,self.target:target,self.click_mask:click_mask})
        


def check_win(truthboard,clickboard):
    Nclickinds=np.where(clickboard==0)
    mineinds=np.where(truthboard>=16)

    return np.array_equal(Nclickinds[0],mineinds[0]) and np.array_equal(Nclickinds[1],mineinds[1])
        
if __name__=='__main__':
    net=FCN()
    trials=12500
    reward={1:-.075,2:-.075,0:0.025}
    alpha=0.1
    score=0
    board=BoardPy.BoardPy(10,10)
    meanscores=[]
    curmean=[]
    numwins=0
    lamb=0.5
    outdir='/mnt/D2/Minesweeper/'
    for i in range(trials):
        if i%2000==0:alpha=alpha/10
        clickmask=np.ones((1,10,10,1))
        end=False
        if i!=0 and i%10==0:
            print(i)
            meanscores.append(np.mean(curmean))
            curmean=[]
        rounda=0
        while not end:
            if (check_win(board.truthboard,board.clickboard)):
                numwins+=1
            #    break
            curboard=discboard(board.mineboard)#np.array(board.mineboard).reshape([1,10,10,1])
            prob=net.predict_prob(curboard,clickmask)

            rng=np.random.random()
            if rng < alpha:
                inds=np.where(clickmask==1)
                choice=np.random.choice(len(inds),1)
                proposed_click=(0,inds[1][choice],inds[2][choice],0)
            else:
                proposed_click=np.unravel_index(np.argmax(prob),(1,10,10,1))
            #print(prob,proposed_click)
            result=board.click(proposed_click[1],proposed_click[2])
            rew=reward[result]#*(1+rounda*0.05)
            #if (result==2 and rounda==0):
            #    break
            nextboard=discboard(board.mineboard)
            nextQ=net.predict_prob(nextboard,clickmask)
            #nextQ[proposed_click]+=rew
            if result==2:
                prob[proposed_click]+=rew
            else:
                nextQ[proposed_click]=0
                prob[proposed_click]=rew+lamb*np.max(nextQ) #0 out proposed_click
            net.update(curboard,clickmask,prob)
            score+=1
            #clickmask[proposed_click]=0
            #if not np.any(clickmask): end=True
            
            clickmask=np.abs(np.array(board.clickboard).astype(np.float32)-1).reshape((1,10,10,1))
            #clickmask=np.abs(clickboard).reshape((1,10,10,1))
            if result==2:end=True
            rounda+=1
        #rounda=board.score()
        curmean.append(rounda)
        board.remake(10,10)
    end=False
    print('num wins',numwins)
    numwins=0
    import imageio
    for i in range(10):
        board.remake(10,10)
        frames=[]
        clickmask=np.ones((1,10,10,1))
        rounda=0
        end=False
        
        while not end:
            #if (check_win(board.truthboard,board.clickboard)):
                #numwins+=1
            #    break
                
            curboard=discboard(board.mineboard)
            prob=net.predict_prob(curboard,clickmask)
            proposed_click=np.unravel_index(np.argmax(prob),(1,10,10,1))
            result=board.click(proposed_click[1],proposed_click[2])
            clickmask=np.abs(np.array(board.clickboard).astype(np.float32)-1).reshape((1,10,10,1))
            #clickmask[proposed_click]=0
            print(proposed_click)
            print(prob)
            if result==2 and rounda==0:
                board.remake(10,10)
                clickmask=np.ones((1,10,10,1))
                frames=[]
                continue
            if result==2:end=True
            img=np.array(board.imgboard.view(np.uint8).reshape(10*32,-1,4))
            frames.append(img)
            rounda+=1
        print('Frames:',len(frames))
        imageio.mimsave(outdir+'run_'+str(trials)+'_'+str(i)+'.gif',frames,duration=1)

        import matplotlib.pyplot as plt
        plt.plot(np.arange(0,len(meanscores)),meanscores)
        plt.savefig(outdir+'scores_'+str(trials)+'_'+str(i)+'.png')
        plt.clf()
    
    
##Try true TD (on-policy)
