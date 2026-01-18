const express = require("express");
const session = require("express-session");
const { MongoClient } = require("mongodb");

const app = express();
const PORT = 3000;

const MONGO_URL = "mongodb://localhost:27017";
const DB_NAME = "smarthome";
let db;

// ===== View Engine =====
app.set("view engine", "ejs");
app.use(express.urlencoded({ extended: true }));
app.use(express.static("public"));

app.use(
  session({
    secret: "iot-secret",
    resave: false,
    saveUninitialized: false
  })
);

// ===== Auth Middleware =====
function requireAuth(req, res, next) {
  if (!req.session.user) return res.redirect("/login");
  next();
}

// ===== Routes =====
app.get("/login", (req, res) => res.render("login"));

app.post("/login", async (req, res) => {
  const { username, password } = req.body;
  const user = await db.collection("users").findOne({ username, password });

  if (user) {
    req.session.user = user.username;
    res.redirect("/dashboard");
  } else {
    res.send("Invalid credentials");
  }
});

app.get("/dashboard", requireAuth, async (req, res) => {
  const latest = await db
    .collection("iot_data")
    .find()
    .sort({ timestamp: -1 })
    .limit(1)
    .toArray();

  res.render("dashboard", { data: latest[0] || {} });
});

app.get("/api/latest", requireAuth, async (req, res) => {
  const latest = await db
    .collection("iot_data")
    .find()
    .sort({ timestamp: -1 })
    .limit(1)
    .toArray();

  res.json(latest[0] || {});
});

app.get("/logout", (req, res) => {
  req.session.destroy();
  res.redirect("/login");
});

// ===== Start Server =====
(async () => {
  const client = await MongoClient.connect(MONGO_URL);
  db = client.db(DB_NAME);
  console.log("[MongoDB] Connected");

  app.listen(PORT, () =>
    console.log(`Dashboard running on http://localhost:${PORT}`)
  );
})();
