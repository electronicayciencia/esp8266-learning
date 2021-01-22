% Poor-man's logic analyzer

data = load("D:\tmp\data.csv");
time = data(:,1);
pin = data(:,2);
level = data(:,3);

stairs(time, level);
ylim([-0.01, 1.1]);
xlabel("Time (\mus)");
ylabel("Logic level");
