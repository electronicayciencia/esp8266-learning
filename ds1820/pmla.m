% Poor-man's logic analyzer

data = load("D:\tmp\data.csv");
time = data(:,1);
level = data(:,2);

stairs(time, level);
ylim([-0.01, 1.1]);
xlabel("Time (\mus)");
ylabel("Logic level");
