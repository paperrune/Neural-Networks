import numpy as np
import tensorflow as tf
import time
 
batch_size = 128
epochs = 30
learning_rate = 0.05
momentum = 0.9
 
# input image dimensions
img_rows, img_cols = 28, 28
 
(x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()
 
x_train = x_train.reshape([x_train.shape[0], img_rows, img_cols, 1]).astype('float32') / 255
y_train = tf.keras.utils.to_categorical(y_train, num_classes=10)
 
x_test = x_test.reshape([x_test.shape[0], img_rows, img_cols, 1]).astype('float32') / 255
y_test = tf.keras.utils.to_categorical(y_test, num_classes=10)
 
# input placeholders
X = tf.placeholder(tf.float32, [None, img_rows, img_cols, 1])
Y = tf.placeholder(tf.float32, [None, 10])
 
# weights & bias for networks
W1 = tf.get_variable(name="W1", shape=[5, 5, 1, 24], initializer=tf.keras.initializers.he_normal())
b1 = tf.get_variable(name="b1", shape=[24], initializer=tf.zeros_initializer())
L1 = tf.nn.conv2d(X, W1, strides=[1, 1, 1, 1], padding='VALID')
L1 = tf.nn.relu(tf.nn.bias_add(L1, b1))
P1 = tf.nn.max_pool(L1, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='SAME')

# depthwise
W2_depthwise = tf.get_variable(name="W2_depthwise", shape=[5, 5, 24, 1], initializer=tf.keras.initializers.he_normal())
b2_depthwise = tf.get_variable(name="b2_depthwise", shape=[24], initializer=tf.zeros_initializer())
L2 = tf.nn.depthwise_conv2d(P1, W2_depthwise, strides=[1, 1, 1, 1], padding='VALID')
L2 = tf.nn.relu(tf.nn.bias_add(L2, b2_depthwise))

# pointwise
W2_pointwise = tf.get_variable(name="W2_pointwise", shape=[1, 1, 24, 48], initializer=tf.keras.initializers.he_normal())
b2_pointwise = tf.get_variable(name="b2_pointwise", shape=[48], initializer=tf.zeros_initializer())
L2 = tf.nn.conv2d(L2, W2_pointwise, strides=[1, 1, 1, 1], padding='VALID')
L2 = tf.nn.relu(tf.nn.bias_add(L2, b2_pointwise))

P2 = tf.nn.max_pool(L2, ksize=[1, 2, 2, 1], strides=[1, 2, 2, 1], padding='SAME')

P2_flat = tf.reshape(P2, [-1, 4 * 4 * 48])
 
W3 = tf.get_variable(name="W3", shape=[4 * 4 * 48, 512], initializer=tf.keras.initializers.he_normal())
b3 = tf.get_variable(name="b3", shape=[512], initializer=tf.zeros_initializer())
L3 = tf.nn.relu(tf.matmul(P2_flat, W3) + b3)
 
W4 = tf.get_variable(name="W4", shape=[512, 10], initializer=tf.glorot_uniform_initializer())
b4 = tf.get_variable(name="b4", shape=[10], initializer=tf.zeros_initializer())
 
hypothesis = tf.matmul(L3, W4) + b4
 
# define cost & optimizer
cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=hypothesis, labels=Y))
optimizer = tf.train.MomentumOptimizer(learning_rate=learning_rate, momentum=momentum, use_nesterov=True)
train = optimizer.minimize(cost)
 
accuracy = tf.reduce_mean(tf.cast(tf.equal(tf.argmax(hypothesis, 1), tf.argmax(Y, 1)), tf.float32))
 
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    start_time = time.time()
 
    for step in range(epochs):
        # shuffle data
        p = np.random.permutation(len(x_train))
        x_train = x_train[p]
        y_train = y_train[p]
        
        loss = [0, 0]
        score = [0, 0]
 
        for i in range(0, x_train.shape[0], batch_size):
            x_batch = x_train[i:i + batch_size]
            y_batch = y_train[i:i + batch_size]
            
            c, a, _ = sess.run([cost, accuracy, train], feed_dict={X: x_batch, Y: y_batch})
            loss[0] += c * len(x_batch)
            score[0] += a * len(x_batch)
 
        for i in range(0, x_test.shape[0], batch_size):
            x_batch = x_test[i:i + batch_size]
            y_batch = y_test[i:i + batch_size]
            
            c, a = sess.run([cost, accuracy], feed_dict={X: x_batch, Y: y_batch})
            loss[1] += c * len(x_batch)
            score[1] += a * len(x_batch)
            
        print('loss: {:.4f} / {:.4f}\taccuracy: {:.4f} / {:.4f}\tstep {}  {:.2f} sec'.format(loss[0] / x_train.shape[0], loss[1] / x_test.shape[0], score[0] / x_train.shape[0], score[1] / x_test.shape[0], step + 1, time.time() - start_time))
