import tensorflow as tf
import time

batch_size = 128
epochs = 100
learning_rate = 0.005
momentum = 0.9
time_step = 28

(x_train, y_train), (x_test, y_test) = tf.keras.datasets.mnist.load_data()

x_train = x_train.reshape([x_train.shape[0], time_step, 784 // time_step]).astype('float32') / 255
y_train = tf.keras.utils.to_categorical(y_train, num_classes=10)

x_test = x_test.reshape([x_test.shape[0], time_step, 784 // time_step]).astype('float32') / 255
y_test = tf.keras.utils.to_categorical(y_test, num_classes=10)

# input place holders
X = tf.placeholder(tf.float32, [None, time_step, 784 // time_step])
Y = tf.placeholder(tf.float32, [None, 10])

# weights & bias for networks
with tf.variable_scope('rnn', initializer=tf.keras.initializers.he_normal()):
    cell = tf.nn.rnn_cell.BasicRNNCell(128, activation=tf.nn.relu)
    outputs, states = tf.nn.dynamic_rnn(cell, X, dtype=tf.float32)
    outputs = tf.transpose(outputs, [1, 0, 2])[-1]
 
W = tf.get_variable(name="W", shape=[128, 10], initializer=tf.initializers.glorot_uniform())
b = tf.get_variable(name="b", shape=[10], initializer=tf.zeros_initializer())
 
hypothesis = tf.matmul(outputs, W) + b

# cost & optimizer
cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(logits=hypothesis, labels=Y))
optimizer = tf.train.MomentumOptimizer(learning_rate=learning_rate, momentum=momentum, use_nesterov=True)
train = optimizer.minimize(cost)

accuracy = tf.reduce_mean(tf.cast(tf.equal(tf.argmax(hypothesis, 1), tf.argmax(Y, 1)), tf.float32))

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    start_time = time.time()

    for step in range(epochs):
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
