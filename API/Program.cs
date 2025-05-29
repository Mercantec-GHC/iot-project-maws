using DomainModels;
using API.Data;
using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.EntityFrameworkCore;
using Microsoft.IdentityModel.Tokens;
using System.Net.NetworkInformation;
using System.Text;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddControllers();
// Learn more about configuring Swagger/OpenAPI at https://aka.ms/aspnetcore/swashbuckle
builder.Services.AddEndpointsApiExplorer();
builder.Services.AddSwaggerGen();

// Connection to database.
builder.Services.AddDbContext<AppDBContext>(options => options.UseNpgsql(builder.Configuration.GetConnectionString("DefaultConnection")));

// Configure JWT authentication with relaxed HTTPS requirements for development
builder.Services.AddAuthentication(JwtBearerDefaults.AuthenticationScheme)
    .AddJwtBearer(options =>
    {
        options.TokenValidationParameters = new TokenValidationParameters
        {
            ValidateIssuer = true,
            ValidateAudience = true,
            ValidateLifetime = true,
            ValidateIssuerSigningKey = true,
            ValidIssuer = builder.Configuration["Jwt:Issuer"],
            ValidAudience = builder.Configuration["Jwt:Audience"],
            IssuerSigningKey = new SymmetricSecurityKey(Encoding.UTF8.GetBytes(builder.Configuration["Jwt:Key"]!))
        };

        // Allow HTTP requests in development
        if (builder.Environment.IsDevelopment())
        {
            options.RequireHttpsMetadata = false;
        }
    });

// Add CORS configuration - allow both HTTP and HTTPS
builder.Services.AddCors(options =>
{
    options.AddPolicy("AllowAllOrigins", builder =>
    {
        builder.WithOrigins(
                "https://localhost:7091",
                "http://localhost:5021",
                "https://localhost:7207",
                "http://192.168.1.16:5021",
                "https://192.168.1.16:7207",
                "http://localhost:5143" // Add your Blazor HTTP endpoint
            )
            .AllowAnyMethod()
            .AllowAnyHeader()
            .AllowCredentials(); // Important for JWT cookies
    });
});

var app = builder.Build();

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

// Conditional HTTPS redirection - only redirect browser requests, not API calls
app.Use(async (context, next) =>
{
    // Skip HTTPS redirection for API endpoints to allow Arduino HTTP calls
    if (!context.Request.Path.StartsWithSegments("/api") && 
        !context.Request.IsHttps && 
        !app.Environment.IsDevelopment())
    {
        var httpsUrl = $"https://{context.Request.Host}{context.Request.PathBase}{context.Request.Path}{context.Request.QueryString}";
        context.Response.Redirect(httpsUrl);
        return;
    }
    await next();
});

app.UseCors("AllowAllOrigins");

app.UseAuthentication();
app.UseAuthorization();

app.MapControllers();

app.Run();